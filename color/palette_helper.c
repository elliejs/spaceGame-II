#include "palette_helper.h"

extern inline
unsigned int closest_color_index(oklab_t color, oklab_t * colors, unsigned int num_colors);

// extern inline
// SGVecOKLAB_t normalize_SGVecOKLAB(SGVecOKLAB_t color);

extern inline
bool create_gradient(oklab_t start, oklab_t end, unsigned int steps, unsigned int max_colors, oklab_t * color_storage, unsigned int * num_colors, bool install_ends);

extern inline
bool install_color(oklab_t color, unsigned int max_colors, oklab_t * color_storage, unsigned int * num_colors);
