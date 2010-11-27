#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "Efreet.h"
#include "efreet_private.h"
#include "efreet_cache_private.h"

static const char *exts[] = { ".png", ".xpm", ".svg", NULL };

static int verbose = 0;

static Eina_Bool
_cache_extention_lookup(const char *ext)
{
    unsigned int i;

    for (i = 0; exts[i]; ++i)
        if (!strcmp(exts[i], ext))
            return EINA_TRUE;
    return EINA_FALSE;
}

static Eina_Bool
cache_fallback_scan_dir(Eina_Hash *icons, Eina_Hash *dirs, const char *dir, Eina_Bool *changed)
{
    Eina_Iterator *it;
    Eina_File_Direct_Info *entry;

    if (eina_hash_find(dirs, dir)) return 1;
    eina_hash_add(dirs, dir, (void *)-1);

    it = eina_file_stat_ls(dir);
    if (!it) return 1;

    EINA_ITERATOR_FOREACH(it, entry)
    {
        Efreet_Cache_Fallback_Icon *icon;
        char *name;
        char *ext;
        unsigned int i;

        if (entry->type == EINA_FILE_DIR)
            continue;

        ext = strrchr(entry->path + entry->name_start, '.');
        if (!ext || !_cache_extention_lookup(ext))
            continue;

        /* icon with known extension */
        name = entry->path + entry->name_start;
        *ext = '\0';

        icon = eina_hash_find(icons, name);
        if (!icon)
        {
            icon = NEW(Efreet_Cache_Fallback_Icon, 1);
            icon->free = 1;
            icon->theme = NULL;
            eina_hash_add(icons, name, icon);
        }

        *ext = '.';

        for (i = 0; i < icon->icons_count; ++i)
            if (!strcmp(icon->icons[i], entry->path))
                break;

        if (i != icon->icons_count)
            continue ;

        /* we don't really track path deat here, so we will leak... */
        icon->icons = realloc(icon->icons, sizeof (char *) * (icon->icons_count + 1));
        icon->icons[icon->icons_count++] = eina_stringshare_add(entry->path);

        *changed = EINA_TRUE;
    }

    eina_iterator_free(it);

    return 1;
}

static Eina_Bool
cache_fallback_scan(Eina_Hash *icons, Eina_Bool *changed)
{
    Eina_List *xdg_dirs, *l;
    const char *dir;
    char path[PATH_MAX];
    Eina_Hash *dirs;

    dirs = eina_hash_string_superfast_new(NULL);

    cache_fallback_scan_dir(icons, dirs, efreet_icon_deprecated_user_dir_get(), changed);
    cache_fallback_scan_dir(icons, dirs, efreet_icon_user_dir_get(), changed);

    xdg_dirs = efreet_data_dirs_get();
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/icons", dir);
        cache_fallback_scan_dir(icons, dirs, path, changed);
    }

#ifndef STRICT_SPEC
    EINA_LIST_FOREACH(xdg_dirs, l, dir)
    {
        snprintf(path, sizeof(path), "%s/pixmaps", dir);
        cache_fallback_scan_dir(icons, dirs, path, changed);
    }
#endif

    cache_fallback_scan_dir(icons, dirs, "/usr/share/pixmaps", changed);

    eina_hash_free(dirs);
    return 1;
}

static Eina_Bool
cache_scan_path_dir(Efreet_Icon_Theme *theme,
                    const char *path,
                    Efreet_Icon_Theme_Directory *dir,
                    Eina_Hash *icons,
                    Eina_Bool *changed)
{
    Eina_Iterator *it;
    char buf[PATH_MAX];
    Eina_File_Direct_Info *entry;

    snprintf(buf, sizeof(buf), "%s/%s", path, dir->name);

    it = eina_file_stat_ls(buf);
    if (!it) return 1;

    EINA_ITERATOR_FOREACH(it, entry)
    {
        Efreet_Cache_Icon *icon;
        char *name;
        char *ext;
        unsigned int i;

        if (entry->type == EINA_FILE_DIR)
            continue;

        ext = strrchr(entry->path + entry->name_start, '.');
        if (!ext || !_cache_extention_lookup(ext))
            continue;

        /* icon with known extension */
        name = entry->path + entry->name_start;
        *ext = '\0';

        icon = eina_hash_find(icons, name);
        if (!icon)
        {
            icon = NEW(Efreet_Cache_Icon, 1);
            icon->free = 1;
            icon->theme = eina_stringshare_add(theme->name.internal);
            eina_hash_add(icons, name, icon);
        }
        else if (icon->theme && strcmp(icon->theme, theme->name.internal))
        {
            /* We got this icon from a parent theme */
            continue;
        }

        /* find if we have the same icon in another type */
        for (i = 0; i < icon->icons_count; ++i)
        {
            if ((icon->icons[i]->type == dir->type) &&
                (icon->icons[i]->normal == dir->size.normal) &&
                (icon->icons[i]->max == dir->size.max) &&
                (icon->icons[i]->min == dir->size.min))
                break;
        }

        *ext = '.';

        if (i != icon->icons_count)
        {
            unsigned int j;

            /* check if the path already exist */
            for (j = 0; j < icon->icons[i]->paths_count; ++j)
                if (!strcmp(icon->icons[i]->paths[j], entry->path))
                    break ;

            if (j != icon->icons[i]->paths_count)
                continue ;
        }
        /* no icon match so add a new one */
        else
        {
            icon->icons = realloc(icon->icons,
                                  sizeof (Efreet_Cache_Icon_Element*) * (++icon->icons_count));
            icon->icons[i] = NEW(Efreet_Cache_Icon_Element, 1);
            icon->icons[i]->type = dir->type;
            icon->icons[i]->normal = dir->size.normal;
            icon->icons[i]->min = dir->size.min;
            icon->icons[i]->max = dir->size.max;
            icon->icons[i]->paths = NULL;
            icon->icons[i]->paths_count = 0;
        }

        /* and finally store the path */
        icon->icons[i]->paths = realloc(icon->icons[i]->paths,
                                        sizeof (char*) * (icon->icons[i]->paths_count + 1));
        icon->icons[i]->paths[icon->icons[i]->paths_count++] = eina_stringshare_add(entry->path);

        *changed = EINA_TRUE;
    }

    eina_iterator_free(it);

    return 1;
}

static int
cache_scan_path(Efreet_Icon_Theme *theme, Eina_Hash *icons, const char *path, Eina_Bool *changed)
{
    Eina_List *l;
    Efreet_Icon_Theme_Directory *dir;

    EINA_LIST_FOREACH(theme->directories, l, dir)
        if (!cache_scan_path_dir(theme, path, dir, icons, changed)) return 0;

    return 1;
}

static Eina_Bool
cache_scan(Efreet_Icon_Theme *theme, Eina_Hash *themes, Eina_Hash *icons, Eina_Bool *changed)
{
    Eina_List *l;
    const char *path;
    const char *name;

    if (!theme) return 1;
    if (eina_hash_find(themes, theme->name.internal)) return 1;
    eina_hash_direct_add(themes, theme->name.internal, theme);

    /* scan theme */
    EINA_LIST_FOREACH(theme->paths, l, path)
        if (!cache_scan_path(theme, icons, path, changed)) return 0;

    /* scan inherits */
    if (theme->inherits)
    {
        EINA_LIST_FOREACH(theme->inherits, l, name)
        {
            Efreet_Icon_Theme *inherit;

            inherit = efreet_icon_theme_find(name);
            if (!inherit) fprintf(stderr, "Theme `%s` not found for `%s`.\n",
                                  name, theme->name.internal);
            if (!cache_scan(inherit, themes, icons, changed)) return 0;
        }
    }
    else if (strcmp(theme->name.internal, "hicolor"))
    {
        theme = efreet_icon_theme_find("hicolor");
        if (!cache_scan(theme, themes, icons, changed)) return 0;
    }

    return 1;
}

static int
cache_lock_file(void)
{
    char file[PATH_MAX];
    struct flock fl;
    int lockfd;

    snprintf(file, sizeof(file), "%s/.efreet/icon_data.lock", efreet_home_dir_get());
    lockfd = open(file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (lockfd < 0) return -1;

    memset(&fl, 0, sizeof(struct flock));
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    if (fcntl(lockfd, F_SETLK, &fl) < 0)
    {
        if (verbose) printf("LOCKED! You may want to delete %s if this persists\n", file);
        close(lockfd);
        return -1;
    }

    return lockfd;
}

static void
term_handler(int sig __UNUSED__, siginfo_t * info __UNUSED__, void *data __UNUSED__)
{
}

/* prevent early death */
static void
catch_sigterm(void)
{
    struct sigaction act;

    act.sa_sigaction = term_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGTERM, &act, NULL) < 0)
        perror("sigaction"); /* It's bad if we can't deal with SIGTERM, but not dramatic */
}

int
main(int argc, char **argv)
{
    /* TODO:
     * - Add file monitor on files, so that we catch changes on files
     *   during whilst this program runs.
     * - Maybe linger for a while to reduce number of cache re-creates.
     * - pass extra dirs to binary, and read them
     * - make sure programs with different extra dirs all work together
     */
    Efreet_Cache_Theme *cache;
    Efreet_Icon_Theme *theme;
    Eet_Data_Descriptor *edd;
    Eet_File *ef;
    Eina_List *l = NULL;
    char file[PATH_MAX];
    char *dir = NULL;
    Eina_Bool changed = EINA_FALSE;
    int lockfd = -1;
    int i;

    for (i = 1; i < argc; i++)
    {
        if      (!strcmp(argv[i], "-v")) verbose = 1;
        else if ((!strcmp(argv[i], "-h")) ||
                 (!strcmp(argv[i], "-help")) ||
                 (!strcmp(argv[i], "--h")) ||
                 (!strcmp(argv[i], "--help")))
        {
            printf("Options:\n");
            printf("  -v     Verbose mode\n");
            exit(0);
        }
    }

    /* init external subsystems */
    if (!eina_init()) return -1;
    if (!eet_init()) return -1;
    if (!ecore_init()) return -1;

    efreet_cache_update = 0;

    /* FIXME: should be in cache dir maybe */
    /* create homedir */
    snprintf(file, sizeof(file), "%s/.efreet", efreet_home_dir_get());
    if (!ecore_file_mkpath(file)) return -1;

    catch_sigterm();

    /* lock process, so that we only run one copy of this program */
    lockfd = cache_lock_file();
    if (lockfd == -1) return -1;

    /* create dir for icon cache */
    dir = ecore_file_dir_get(efreet_icon_cache_file());
    if (!ecore_file_mkpath(dir)) goto on_error;
    free(dir);

    /* finish efreet init */
    if (!efreet_init()) goto on_error;

    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            theme = efreet_icon_theme_find(argv[i]);
            if (theme) l = eina_list_append(l, theme);
        }
    }

    if (!l) l = efreet_icon_theme_list_get();

    ef = eet_open(efreet_icon_cache_file(), EET_FILE_MODE_READ_WRITE);
    if (!ef) goto on_error_efreet;

    edd = efreet_icon_theme_edd();

    EINA_LIST_FREE(l, theme)
    {
        Eina_Hash *themes;

        themes = eina_hash_string_superfast_new(NULL);

        /* read icons from the eet file */
        cache = eet_data_read(ef, edd, theme->name.internal);

        /* Wype out in case of version change */
        if (cache
            && cache->version.major != EFREET_CACHE_MAJOR
            && cache->version.minor != EFREET_CACHE_MINOR)
        {
            eina_hash_free(cache->icons);
            free(cache);
            cache = NULL;
        }

        /* No existing cache before, so create it */
        if (!cache)
        {
            cache = malloc(sizeof (Efreet_Cache_Theme));
            if (!cache) goto on_error_efreet;

            cache->version.major = EFREET_CACHE_MAJOR;
            cache->version.minor = EFREET_CACHE_MINOR;

            cache->icons = eina_hash_string_superfast_new((Eina_Free_Cb) efreet_cache_icon_free);

            changed = EINA_TRUE;
        }

        if (cache_scan(theme, themes, cache->icons, &changed))
        {
            fprintf(stderr, "generated: '%s' %i (%i)\n",
                    theme->name.internal,
                    changed,
                    eina_hash_population(cache->icons));
            if (changed)
                eet_data_write(ef, edd, theme->name.internal, cache, 1);
            changed = EINA_FALSE;
        }

        eina_hash_free(themes);
        eina_hash_free(cache->icons);
        free(cache);
    }

    edd = efreet_icon_fallback_edd();

    /* read fallback icons from the eet file */
    cache = eet_data_read(ef, edd, "efreet/fallback");
    if (cache && cache->version.major != EFREET_CACHE_MAJOR)
    {
        eina_hash_free(cache->icons);
        free(cache);
        cache = NULL;
    }

    /* No existing fallback, create it */
    if (!cache)
    {
        cache = malloc(sizeof (Efreet_Cache_Theme));
        if (!cache) goto on_error_efreet;

        cache->version.major = EFREET_CACHE_MAJOR;
        cache->version.minor = EFREET_CACHE_MINOR;

        cache->icons = eina_hash_string_superfast_new((Eina_Free_Cb) efreet_cache_icon_fallback_free);

        changed = EINA_TRUE;
    }

    /* Save fallback in the right part */
    if (cache_fallback_scan(cache->icons, &changed))
    {
        fprintf(stderr, "generated: fallback %i (%i)\n", changed, eina_hash_population(cache->icons));
        if (changed)
            eet_data_write(ef, edd, "efreet/fallback", cache, 1);
    }

    eina_hash_free(cache->icons);
    free(cache);

    /* save data */
    eet_close(ef);

on_error_efreet:
    efreet_shutdown();

on_error:
    close(lockfd);

    ecore_shutdown();
    eet_shutdown();
    eina_shutdown();

    return 0;
}
