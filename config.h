#include <X11/XF86keysym.h>

/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows */
static const unsigned int snap      = 10;       /* snap pixel */
static const int rmaster            = 1;        /* 1 means master-area is initially on the right */
static const unsigned int gappih    = 10;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 10;       /* vert inner gap between windows */
static const unsigned int gappoh    = 10;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 10;       /* vert outer gap between windows and screen edge */
static       int smartgaps          = 0;        /* 1 means no outer gap when there is only one window */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;     /* 0 means no systray */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int focusonwheel       = 0;
static const char *fonts[]          = { "Fira Mono:size=10" };
static const char dmenufont[]       = "Fira Mono:size=10";
static const char foreground[]      = "#d0d0d0";
static const char background[]      = "#181418";
static const char color0[]          = "#4e4e4e";
static const char color1[]          = "#d68787";
static const char color2[]          = "#5f865f";
static const char color3[]          = "#d8af5f";
static const char color4[]          = "#85add4";
static const char color5[]          = "#d7afaf";
static const char color6[]          = "#87afaf";
static const char color7[]          = "#d0d0d0";
static const char color8[]          = "#626262";
static const char color9[]          = "#d75f87";
static const char color10[]         = "#87af87";
static const char color11[]         = "#ffd787";
static const char color12[]         = "#add4fb";
static const char color13[]         = "#ffafaf";
static const char color14[]         = "#87d7d7";
static const char color15[]         = "#e4e4e4";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { foreground, background, background },
	[SchemeSel]  = { background, color5, color5  },
};

typedef struct {
	const char *name;
	const void *cmd;
} Sp;
const char *spcmd1[] = {"st", "-n", "spterm", NULL };
static Sp scratchpads[] = {
	/* name          cmd  */
	{"spterm",      spcmd1},
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "st-256color", NULL, NULL,           0,         0,          1,           0,        -1 },
	{ NULL, "spterm", NULL,		             SPTAG(0),	1,          1,           0,        -1 },
	{ "Spacefm", NULL,     NULL,           0,         1,          0,           0,        -1 },
	{ "Pavucontrol", NULL, NULL,           0,         1,          0,           0,        -1 },
	{ "gammy", NULL, NULL,                 0,         1,          0,           0,        -1 },
	{ NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.45; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include "vanitygaps.c"

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "[]|",      bstack },
	{ "[]D",      deck },
	{ "[M]",      monocle },
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[=]",      centeredmaster },
	{ "[>]",      centeredfloatingmaster },
	{ NULL,       NULL },
};

//	{ "[@]",      spiral },
//	{ "[\\]",     dwindle },
//	{ "===",      bstackhoriz },
//	{ "HHH",      grid },
//	{ "###",      nrowgrid },
//	{ "---",      horizgrid },
//	{ ":::",      gaplessgrid },

static const MonitorRule monrules[] = {
	/* monitor  tag  layout  mfact  nmaster  showbar  topbar */
	{  1,       -1,  1,      -1,    -1,      -1,      -1     }, // use a different layout for the second monitor
	{  -1,      -1,  0,      -1,    -1,      -1,      -1     }, // default
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
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", background, "-nf", foreground, "-sb", color5, "-sf", background, "-shb", color2, NULL };
static const char *passcmd[] = { "passmenu", "-m", dmenumon, "-fn", dmenufont, NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *browsercmd[]  = { "brave", NULL };
static const char *virtmanagercmd[]  = { "virt-manager", NULL };
static const char *fileexplorercmd[]  = { "spacefm", NULL };
static const char *pavucontrolcmd[]  = { "pavucontrol", NULL };
static const char *takescreencmd[]  = { "takeScreen", NULL };
static const char *downvol[]  = { "pamixer", "-d", "5", NULL };
static const char *upvol[]    = { "pamixer", "-i", "5", NULL };
static const char *mutevol[]    = { "pamixer", "-t", NULL };

#include "movestack.c"

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_s,      spawn,          {.v = passcmd } },
	{ MODKEY|ShiftMask,       	    XK_Return, spawnsshaware,  {.v = termcmd } },
	{ MODKEY|ShiftMask,             XK_i,      spawn,          {.v = browsercmd } },
	{ MODKEY|ShiftMask,             XK_m,      spawn,          {.v = virtmanagercmd} },
	{ MODKEY|ShiftMask,             XK_e,      spawn,          {.v = fileexplorercmd } },
	{ MODKEY|ShiftMask,             XK_p,      spawn,          {.v = pavucontrolcmd} },
	{ MODKEY|ShiftMask,             XK_s,      spawn,          {.v = takescreencmd} },
	{ 0,         XF86XK_AudioLowerVolume,      spawn,          {.v = downvol } },
	{ 0,         XF86XK_AudioRaiseVolume,      spawn,          {.v = upvol   } },
	{ 0,                XF86XK_AudioMute,      spawn,          {.v = mutevol   } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_o,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY|ShiftMask,             XK_g,      togglegaps,     {0} },
	{ MODKEY,                       XK_Tab,    switchcol,      {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_y,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_d,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[4]} },
	{ MODKEY,                       XK_v,      setlayout,      {.v = &layouts[5]} },
	{ MODKEY,                       XK_c,      setlayout,      {.v = &layouts[6]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_r,      togglermaster,  {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY,            			      XK_grave,  togglescratch,  {.ui = 0 } },
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
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {.v = &layouts[0]} },
	{ ClkLtSymbol,          0,              Button2,        setlayout,      {.v = &layouts[4]} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[1]} },
	{ ClkWinTitle,          0,              Button1,        zoom,           {0} },
	{ ClkWinTitle,          0,              Button2,        killclient,     {0} },
	{ ClkWinTitle,          0,              Button3,        togglefloating, {0} },
	{ ClkWinTitle,          0,              4,              focusstack,     {.i = -1 } },
	{ ClkWinTitle,          0,              5,              focusstack,     {.i = +1 } },
	{ ClkWinTitle,          0,              8,              spawn,          {.v = browsercmd } },
	{ ClkWinTitle,          0,              9,              spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkClientWin,         0,              8,              resizemouse,    {0} },
	{ ClkClientWin,         0,              9,              movemouse,      {0} },
	{ ClkStatusText,        0,              4,              spawn,          {.v = upvol } },
	{ ClkStatusText,        0,              5,              spawn,          {.v = downvol } },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button2,        tag,            {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            0,              9,              toggletag,      {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
