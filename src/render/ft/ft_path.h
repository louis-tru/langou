#ifndef __quark_render_ft_path__
#define __quark_render_ft_path__
extern "C" {
#include "./ft_stroke.h"
}
#include "../path.h"

Qk_FT_Outline* qk_ft_outline_create(int points, int contours);
void           qk_ft_outline_destroy(Qk_FT_Outline* ft);
Qk_FT_Outline* qk_ft_outline_convert(const qk::Path* path);
Qk_FT_Error    qk_ft_path_convert(Qk_FT_Outline* outline, qk::Path *out);

#endif