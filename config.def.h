/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

#include "themes/nord.h"
#define _CENTRETITLE 1

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int rmaster            = 1;        /* 1 means master-area is initially on the right */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, 0: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int focusonwheel       = 0;
static const char *fonts[]          = { "JetBrainsMono Nerd Font:style=Regular:pixelsize=14:antialias=true:hinting=true" };
static const char dmenufont[]       = "JetBrainsMono Nerd Font:style=Regular:pixelsize=14:antialias=true:hinting=true";
static const char *colors[][3]      = {
  /*               fg         bg         border   */
  [SchemeNorm] = { normal_fg, normal_bg, normal_bd },
  [SchemeSel]  = { select_fg, select_bg, select_bd },
};

typedef struct {
  const char *name;
  const void *cmd;
} Sp;
const char *spcmd1[] = {"st", "-n", "spterm", "-g", "80x24", NULL };
const char *spcmd2[] = {"st", "-n", "spmixer", "-g", "80x24", "-e", "pulsemixer", NULL };
static Sp scratchpads[] = {
  /* name          cmd  */
  {"spterm",      spcmd1},
  {"spmixer",     spcmd2},
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const Rule rules[] = {
  /* xprop(1):
   *	WM_CLASS(STRING) = instance, class
   *	WM_NAME(STRING) = title
   */
  /* class     instance   title           tags mask  isfloating  isterminal  noswallow  monitor */
  { "st-256color", NULL,  NULL,           0,         0,          1,           0,        -1 },
  { NULL,      "spterm",  NULL,           SPTAG(0),  1,          1,           0,        -1 },
  { NULL,      "spmixer", NULL,           SPTAG(1),  1,          0,           0,        -1 },
  { NULL,      NULL,      "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.45; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 0; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
  /* symbol     arrange function */
  { "[]=",      tile },    /* first entry is default */
  { "[D]",      deck },
  { "[M]",      monocle },
  { "><>",      NULL },    /* no layout function means floating behavior */
};

static const MonitorRule monrules[] = {
  /* monitor  tag  layout  rmaster  mfact  nmaster  showbar  topbar */
  {  1,       -1,  2,      -1,      -1,    -1,      -1,      -1     }, // use a different layout for the second monitor
  {  -1,      -1,  0,      -1,      -1,    -1,      -1,      -1     }, // default
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = {
  "dmenu_run", "-m",   dmenumon,  "-fn",  dmenufont, "-nb",
  normal_bg,   "-nf",  normal_fg, "-sb",  select_bg, "-sf",
  select_fg,   "-shb", normal_bd, "-shf", select_bd, NULL};
static const char *pascmd[] = {
  "passmenu", "-m",   dmenumon,  "-fn",  dmenufont, "-nb",
  normal_bg,  "-nf",  normal_fg, "-sb",  select_bg, "-sf",
  select_fg,  "-shb", normal_bd, "-shf", select_bd, NULL};
static const char *tercmd[] = { "st", NULL };
static const char *tabcmd[] = { "tabbed", "-c", "-r", "2", "st", "-w", "''", NULL };
static const char *brwcmd[] = { "firefox-developer-edition", NULL };
static const char *prwcmd[] = { "firefox-developer-edition", "--private-window", NULL };
static const char *vrtcmd[] = { "virt-manager", NULL };
static const char *expcmd[] = { "pcmanfm", NULL };
static const char *scrcmd[] = { "take_screen", NULL };
static const char *incvol[] = { "pulsemixer", "--change-volume", "+5", NULL };
static const char *decvol[] = { "pulsemixer", "--change-volume", "-5", NULL };
static const char *mutvol[] = {"pulsemixer", "--toggle-mute", NULL};
static const char *mutmic[] = {"pulsemixer_micmute", NULL};
static const char *inclit[] = {"light", "-A", "10", NULL};
static const char *declit[] = {"light", "-U", "10", NULL};
static const char *set_bg[] = {"set_bg", NULL};

#include "movestack.c"
static Key keys[] = {
  /* modifier                     key        function        argument */
  { MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
  { MODKEY,                       XK_s,      spawn,          {.v = pascmd } },
  { MODKEY|ShiftMask,             XK_Return, spawn,          {.v = tabcmd } },
  { MODKEY|ShiftMask,             XK_i,      spawn,          {.v = brwcmd } },
  { MODKEY|ShiftMask,             XK_p,      spawn,          {.v = prwcmd } },
  { MODKEY|ShiftMask,             XK_v,      spawn,          {.v = vrtcmd } },
  { MODKEY|ShiftMask,             XK_e,      spawn,          {.v = expcmd } },
  { MODKEY|ShiftMask,             XK_s,      spawn,          {.v = scrcmd } },
  { 0,         XF86XK_AudioRaiseVolume,      spawn,          {.v = incvol } },
  { 0,         XF86XK_AudioLowerVolume,      spawn,          {.v = decvol } },
  { 0,                XF86XK_AudioMute,      spawn,          {.v = mutvol } },
  { 0,             XF86XK_AudioMicMute,      spawn,          {.v = mutmic } },
  { 0,          XF86XK_MonBrightnessUp,      spawn,          {.v = inclit } },
  { 0,        XF86XK_MonBrightnessDown,      spawn,          {.v = declit } },
  { MODKEY|ShiftMask,             XK_b,      spawn,          {.v = set_bg } },
  { MODKEY,                       XK_b,      togglebar,      {0} },
  { MODKEY,                       XK_Return, zoom,           {0} },
  { MODKEY,                       XK_space,  switchcol,      {0} },
  { MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
  { MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
  { MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
  { MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
  { MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
  { MODKEY,                       XK_o,      incnmaster,     {.i = -1 } },
  { MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
  { MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
  { MODKEY|ShiftMask,             XK_h,      setcfact,       {.f = -0.05} },
  { MODKEY|ShiftMask,             XK_l,      setcfact,       {.f = +0.05} },
  { MODKEY|ShiftMask,             XK_o,      setcfact,       {.f =  0.00} },
  { MODKEY,                       XK_r,      togglermaster,  {0} },
  { MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
  { MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
  { MODKEY,                       XK_d,      setlayout,      {.v = &layouts[1]} },
  { MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
  { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[3]} },
  { MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
  { MODKEY,                       XK_grave,  togglescratch,  {.ui = 0 } },
  { MODKEY|ShiftMask,             XK_m,      togglescratch,  {.ui = 1 } },
  { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
  { MODKEY,                       XK_period, focusmon,       {.i = +1 } },
  { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
  { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
  { MODKEY,                       XK_Tab,    view,           {0} },
  { MODKEY,                       XK_0,      view,           {.ui = ~0 } },
  { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
  TAGKEYS(                        XK_1,                      0)
  TAGKEYS(                        XK_2,                      1)
  TAGKEYS(                        XK_3,                      2)
  TAGKEYS(                        XK_4,                      3)
  TAGKEYS(                        XK_5,                      4)
  TAGKEYS(                        XK_6,                      5)
  TAGKEYS(                        XK_7,                      6)
  TAGKEYS(                        XK_8,                      7)
  TAGKEYS(                        XK_9,                      8)
  { MODKEY|ControlMask|ShiftMask, XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
  /* click                event mask      button          function        argument */
  { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
  { ClkLtSymbol,          0,              Button2,        setlayout,      {.v = &layouts[2]} },
  { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[3]} },
  { ClkWinTitle,          0,              Button1,        zoom,           {0} },
  { ClkWinTitle,          0,              Button2,        killclient,     {0} },
  { ClkWinTitle,          0,              Button3,        togglefloating, {0} },
  { ClkWinTitle,          0,              Button4,        focusstack,     {.i = +1 } },
  { ClkWinTitle,          0,              Button5,        focusstack,     {.i = -1 } },
  { ClkStatusText,        0,              Button1,        spawn,          {.v = expcmd } },
  { ClkStatusText,        0,              Button2,        spawn,          {.v = tercmd } },
  { ClkStatusText,        0,              Button3,        spawn,          {.v = brwcmd } },
	{ ClkStatusText,        0,              Button4,        spawn,          {.v = incvol } },
	{ ClkStatusText,        0,              Button5,        spawn,          {.v = decvol } },
  { ClkTagBar,            0,              Button1,        view,           {0} },
  { ClkTagBar,            0,              Button2,        tag,            {0} },
  { ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkClientWin,         0,              8,              resizemouse,    {0} },
	{ ClkClientWin,         0,              9,              movemouse,      {0} },
  { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
  { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
  { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
};
