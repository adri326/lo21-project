#ifndef COLORS_H
#define COLORS_H

#ifndef NO_COLOR
    #define WHITE(str) "\e[97m" str "\e[39m"
    #define GRAY(str) "\e[90m" str "\e[39m"
    #define BLUE(str) "\e[94m" str "\e[39m"
    #define CYAN(str) "\e[36m" str "\e[39m"
    #define GREEN(str) "\e[92m" str "\e[39m"
    #define RED(str) "\e[91m" str "\e[39m"
    #define MAGENTA(str) "\e[95m" str "\e[39m"
#else
    #define WHITE(str) str
    #define GRAY(str) str
    #define BLUE(str) str
    #define CYAN(str) str
    #define GREEN(str) str
    #define RED(str) str
    #define MAGENTA(str) str
#endif

#endif // COLORS_H
