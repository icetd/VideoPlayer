#ifndef SELECT_THEME_H
#define SELECT_THEME_H

class StyleManager {
public:
    typedef enum {
        CLASSIC_STYLE = 0,
        DARK_STYLE,
        LIGHT_STYLE,
        ALTERNATIVE_DARK,
        CINDER,
        HAZEL_DARK,
        DARKNESS,
        DRACULA,
        ENEMYMOUSE
    } MStyle_t;

    static bool ShowStyleSelector(const char* label);
    static void SelectTheme(MStyle_t p_style);
};


#endif