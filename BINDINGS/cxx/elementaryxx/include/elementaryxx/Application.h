#ifndef ELMXX_APPLICATION_H
#define ELMXX_APPLICATION_H

/* STL */
#include <string>

/* EFL++ */

namespace Elmxx {

class Application
{
public:
  Application (int argc, char **argv);
  virtual ~Application ();

  static void run ();
  static void exit ();
  
  // EAPI void         elm_need_efreet(void);
  // EAPI void         elm_need_e_dbus(void);
  
  double getScale ();
  void setScale (double scale);
  
   // EAPI Evas_Coord   elm_finger_size_get(void);
   // EAPI void         elm_finger_size_set(Evas_Coord size);
};

} // end namespace Elmxx

#endif // ELMXX_APPLICATION_H
