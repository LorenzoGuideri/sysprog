#ifndef COLORS_H_INCLUDED
#define COLORS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

struct color {
    unsigned int red;
    unsigned int green;
    unsigned int blue;
};

void string_to_color (struct color *, const char *);

#ifdef __cplusplus
}
#endif

#endif
