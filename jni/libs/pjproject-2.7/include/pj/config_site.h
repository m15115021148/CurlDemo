/* Activate Android specific settings in the 'config_site_sample.h' */  
#define PJ_CONFIG_ANDROID 1  
//To enable video  
#define PJMEDIA_HAS_VIDEO 1  
//To enable libyuv  
#define PJMEDIA_HAS_LIBYUV  1  
//To enable TCP transport  
#define PJ_HAS_TCP 1  
#include <pj/config_site_sample.h>
