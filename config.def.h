#include <X11/XF86keysym.h>

#include "themes/catppuccin.h"

/* appearance */
static const unsigned int borderpx       = 2; /* border pixel of windows */
static const unsigned int snap           = 5; /* snap pixel */
static const int rmaster                 = 1; /* 1 means master-area is initially on the right */
static const int showbar                 = 1; /* 0 means no bar */
static const int topbar                  = 1; /* 0 means bottom bar */
static const int swallowfloating         = 0; /* 1 means swallow floating windows by default */
static const int showsystray             = 1; /* 0 means no systray */
static const unsigned int systraypinning = 0; /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft  = 0; /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 1; /* systray spacing */
static const int systraypinningfailfirst = 1; /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor */
static const char *fonts[]     = { "Iosevka Nerd Font Propo:style=Regular:pixelsize=18" };
static const char dmenufont[]  = "Iosevka Nerd Font Propo::style=Regular:pixelsize=18";
static const char *colors[][3] = {
    /*               fg         bg         border   */
    [SchemeNorm] = { normal_fg, normal_bg, normal_bd },
    [SchemeSel]  = { select_fg, select_bg, select_bd },
};

typedef struct {
    const char *name;
    const void *cmd;
} Sp;
const char *spcmd0[] = { "st", "-n", "spbuff0", "-g", "100x35",  NULL };
const char *spcmd1[] = { "st", "-n", "spbuff1", "-g", "120x40", NULL };
const char *spcmd2[] = { "st", "-n", "spsound", "-g", "100x30",  "-e", "pulsemixer", NULL };
// emacs scratchpad
// const char *spcmd3[] = { "emacs", "--name=spemacs", "-g", "130x50", NULL };
// const char *spcmd3[] = { "emacs", "--name=spemacs", "-g", "130x50", NULL };
static Sp scratchpads[] = {
    /* name           cmd */
    { "spbuff0",      spcmd0 },
    { "spbuff1",      spcmd1 },
    { "spsound",      spcmd2 },
//    { "spemacs",      spcmd3 },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "\\0" };
static const Rule rules[] = {
    /* xprop(1):
     * WM_CLASS(STRING) = instance, class
     * WM_NAME(STRING) = title
    */
    /* class  instance    title            tags mask  isfloating  isterminal  noswallow  monitor */
    { "st",   NULL,       NULL,            0,         0,          1,          0,         -1 },
    { NULL,   "spbuff0",  NULL,            SPTAG(0),  1,          1,          0,         -1 },
    { NULL,   "spbuff1",  NULL,            SPTAG(1),  1,          1,          0,         -1 },
    { NULL,   "spsound",  NULL,            SPTAG(2),  1,          1,          0,         -1 },
    { NULL,   "spemacs",  NULL,            SPTAG(3),  1,          1,          0,         -1 },
    { NULL,   NULL,       "Event Tester",  0,         0,          0,          1,         -1 }, /* xev */
};

/* layout(s) */
static const float mfact        = 0.45; /* factor of master area size [0.05..0.95] */
static const int nmaster        = 1;    /* number of clients in master area */
static const int resizehints    = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 0;    /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
    /* symbol  arrange function */
    { "[]=",   tile },
    { "><>",   NULL },
    { "[M]",   monocle },
    { "[D]",   deck },
    { "###",   nrowgrid },
    { "|M|",   centeredmaster },
    { ">M>",   centeredfloatingmaster },
};

static const MonitorRule monrules[] = {
    /* monitor  tag  layout  mfact  nmaster  showbar  topbar */
    {  -1,       9,  1,      -1,    -1,      -1,      -1     }, // null buffer (tag 9)
    {  -1,      -1,  0,      -1,    -1,      -1,      -1     }, // default
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG)\
    { MODKEY,                       KEY,      view,       { .ui = 1 << TAG } },\
    { MODKEY|Mod1Mask,              KEY,      toggleview, { .ui = 1 << TAG } },\
    { MODKEY|ShiftMask,             KEY,      tag,        { .ui = 1 << TAG } },\
    { MODKEY|ControlMask,           KEY,      toggletag,  { .ui = 1 << TAG } },

/* TODO helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = {
    "dmenu_run", "-m", dmenumon, "-fn", dmenufont,
    "-nb", normal_bg, "-nf", normal_fg,
    "-sb", select_bg, "-sf", select_fg,
    NULL,
};
static const char *clicmd[] = {"st", NULL};
/* static const char *tabcmd[] = {"tabbed", "-k", "-c", "-r", "2", "st", "-w", "''", NULL}; */
static const char *ffbcmd[] = {"firefox", NULL};
static const char *qtbcmd[] = {"qutebrowser", NULL};
static const char *emccmd[] = {"emacsclient", "-a", "emacs --daemon", "-c", NULL};
static const char *scrcmd[] = {"take_screen", NULL};
static const char *incvol[] = {"volume_up", NULL};
static const char *decvol[] = {"volume_down", NULL};
static const char *mutvol[] = {"pulsemixer", "--toggle-mute", NULL};
static const char *mutmic[] = {"pulsemixer_micmute", NULL};
static const char *inclit[] = {"light", "-A", "10", NULL};
static const char *declit[] = {"light", "-U", "10", NULL};
static const char *mpdneg[] = {"mpc", "seekthrough", "-10", NULL};
static const char *mpdtog[] = {"mpc", "toggle", NULL};
static const char *mpdpos[] = {"mpc", "seekthrough", "+10", NULL};

static const Key keys[] = {
    /* modifier                      key                       function        argument */
    {  MODKEY,                       XK_p,                     spawn,          { .v = dmenucmd } },
    {  MODKEY|ShiftMask,             XK_Return,                spawn,          { .v = clicmd } },
    /* {  MODKEY|Mod1Mask,              XK_Return,                spawn,          { .v = tabcmd } }, */
    {  MODKEY|ShiftMask,             XK_i,                     spawn,          { .v = ffbcmd } },
    {  MODKEY|ControlMask,           XK_i,                     spawn,          { .v = qtbcmd } },
    {  MODKEY,                       XK_e,                     spawn,          { .v = emccmd } },
    {  MODKEY|ShiftMask,             XK_s,                     spawn,          { .v = scrcmd } },
    {  0,                            XF86XK_AudioRaiseVolume,  spawn,          { .v = incvol } },
    {  0,                            XF86XK_AudioLowerVolume,  spawn,          { .v = decvol } },
    {  0,                            XF86XK_AudioMute,         spawn,          { .v = mutvol } },
    {  0,                            XF86XK_AudioMicMute,      spawn,          { .v = mutmic } },
    {  0,                            XF86XK_MonBrightnessUp,   spawn,          { .v = inclit } },
    {  0,                            XF86XK_MonBrightnessDown, spawn,          { .v = declit } },
    {  MODKEY,                       XK_Left,                  spawn,          { .v = mpdneg } },
    {  MODKEY,                       XK_Down,                  spawn,          { .v = mpdtog } },
    {  MODKEY,                       XK_Right,                 spawn,          { .v = mpdpos } },
    {  MODKEY,                       XK_b,                     togglebar,      { 0 } },
    {  MODKEY,                       XK_Return,                zoom,           { 0 } },
    {  MODKEY,                       XK_comma,                 switchcol,      { 0 } },
    {  MODKEY,                       XK_j,                     focusstack,     { .i = +1 } },
    {  MODKEY,                       XK_k,                     focusstack,     { .i = -1 } },
    {  MODKEY,                       XK_i,                     incnmaster,     { .i = +1 } },
    {  MODKEY,                       XK_o,                     incnmaster,     { .i = -1 } },
    {  MODKEY,                       XK_h,                     setmfact,       { .f = -0.05 } },
    {  MODKEY,                       XK_l,                     setmfact,       { .f = +0.05 } },
    {  MODKEY|ShiftMask,             XK_h,                     setcfact,       { .f = -0.05 } },
    {  MODKEY|ShiftMask,             XK_l,                     setcfact,       { .f = +0.05 } },
    {  MODKEY|ShiftMask,             XK_o,                     setcfact,       { .f =  0.00 } },
    {  MODKEY|ShiftMask,             XK_j,                     movestack,      { .i = +1 } },
    {  MODKEY|ShiftMask,             XK_k,                     movestack,      { .i = -1 } },
    {  MODKEY|ShiftMask,             XK_c,                     killclient,     { 0 } },
    {  MODKEY,                       XK_t,                     setlayout,      { .v = &layouts[0] } }, // tile
    {  MODKEY,                       XK_f,                     setlayout,      { .v = &layouts[1] } }, // null
    {  MODKEY,                       XK_m,                     setlayout,      { .v = &layouts[2] } }, // monocle
    {  MODKEY,                       XK_d,                     setlayout,      { .v = &layouts[3] } }, // deck
    {  MODKEY,                       XK_g,                     setlayout,      { .v = &layouts[4] } }, // grid
    {  MODKEY,                       XK_c,                     setlayout,      { .v = &layouts[5] } }, // centeredmaster
    {  MODKEY,                       XK_v,                     setlayout,      { .v = &layouts[6] } }, // centeredfloatingmaster
    {  MODKEY,                       XK_r,                     togglermaster,  { 0 } },
    {  MODKEY,                       XK_space,                 setlayout,      { 0 } },
    {  MODKEY|ShiftMask,             XK_space,                 togglefloating, { 0 } },
    // {  MODKEY,                       XK_comma,                 focusmon,       { .i = -1 } },
    {  MODKEY,                       XK_period,                focusmon,       { .i = +1 } },
    // {  MODKEY|ShiftMask,             XK_comma,                 tagmon,         { .i = -1 } },
    {  MODKEY|ShiftMask,             XK_period,                tagmon,         { .i = +1 } },
    {  MODKEY,                       XK_n,                     toggleview,     { .ui = (1<<8) } }, // \0
    {  MODKEY,                       XK_semicolon,             togglescratch,  { .ui = 0 } }, // spbuff0
    {  MODKEY,                       XK_a,                     togglescratch,  { .ui = 1 } }, // spbuff1
    {  MODKEY,                       XK_s,                     togglescratch,  { .ui = 2 } }, // spsound
    {  MODKEY,                       XK_z,                     togglescratch,  { .ui = 3 } }, // spemacs
    {  MODKEY,                       XK_Tab,                   view,           { 0 } },
    {  MODKEY,                       XK_0,                     view,           { .ui = ~0 } },
    {  MODKEY|ShiftMask,             XK_0,                     tag,            { .ui = ~0 } },
    TAGKEYS(                         XK_1,                                       0)
    TAGKEYS(                         XK_2,                                       1)
    TAGKEYS(                         XK_3,                                       2)
    TAGKEYS(                         XK_4,                                       3)
    TAGKEYS(                         XK_5,                                       4)
    TAGKEYS(                         XK_6,                                       5)
    TAGKEYS(                         XK_7,                                       6)
    TAGKEYS(                         XK_8,                                       7)
    TAGKEYS(                         XK_9,                                       8)
    {  MODKEY|ShiftMask,             XK_q,                     quit,           { 0 } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
    /* click             event mask           button      function          argument */
    {  ClkTagBar,        0,                   Button1,    view,             { 0 } },
    {  ClkTagBar,        0,                   Button2,    tag,              { 0 } },
    {  ClkTagBar,        0,                   Button3,    toggleview,       { 0 } },
    {  ClkTagBar,        MODKEY,              Button1,    tag,              { 0 } },
    {  ClkTagBar,        MODKEY,              Button3,    toggletag,        { 0 } },
    {  ClkLtSymbol,      0,                   Button1,    setlayout,        { .v = &layouts[0] } },
    {  ClkLtSymbol,      0,                   Button2,    togglermaster,    { 0 } },
    {  ClkLtSymbol,      0,                   Button3,    setlayout,        { .v = &layouts[2] } },
    {  ClkWinTitle,      0,                   Button1,    zoom,             { 0 } },
    {  ClkWinTitle,      0,                   Button2,    killclient,       { 0 } },
    {  ClkWinTitle,      0,                   Button3,    togglefloating,   { 0 } },
    {  ClkWinTitle,      0,                   Button4,    focusstack,       { .i = +1 } },
    {  ClkWinTitle,      0,                   Button5,    focusstack,       { .i = -1 } },
    {  ClkClientWin,     MODKEY,              Button1,    movemouse,        { 0 } },
    {  ClkClientWin,     MODKEY,              Button2,    togglefloating,   { 0 } },
    {  ClkClientWin,     MODKEY,              Button3,    resizemouse,      { 0 } },
    {  ClkClientWin,     MODKEY|ShiftMask,    Button1,    resizemouse,      { 0 } },
    {  ClkStatusText,    0,                   Button1,    spawn,            { .v = clicmd } },
    {  ClkStatusText,    0,                   Button3,    spawn,            { .v = qtbcmd } },
};
