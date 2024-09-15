/* Portions Copyright 2003, Jordan Crouse (jordan@cosmicpenguin.net) */

#include "nxlib.h"
#include <string.h>
#include "X11/keysym.h"
#include "X11/Xutil.h"
#include "keysymstr.h"

#include <stdlib.h>
#include "uni_std.h"
#include <fcntl.h>
#if !__MINGW32__
#include <sys/ioctl.h>
#endif

#if linux
#include <linux/keyboard.h>
#include <linux/kd.h>
#else
#include "linux/keyboard.h"
#include "linux/kd.h"
#endif

#ifndef __VSF__
static MWKEYMOD modstate;
#else
#define modstate	(vsf_nanox_nx11_ctx->strkeysym._modstate)
#endif

/* Standard keymapings for kernel values */
/* implemented in porting kbd_ttyscan.c */
extern const MWKEY kbd_keymap[128];
MWKEY TranslateScancode(int, MWKEYMOD);

static struct {
	GR_KEY nxKey;
	KeySym xKey;
} const mwkey_to_xkey[] = {
	{MWKEY_RIGHT, XK_Right},
	{MWKEY_LEFT, XK_Left},
	{MWKEY_UP, XK_Up},
	{MWKEY_DOWN, XK_Down},
	{MWKEY_PAGEDOWN, XK_Page_Down},
	{MWKEY_PAGEUP, XK_Page_Up},
	{MWKEY_INSERT, XK_Insert},
	{MWKEY_DELETE, XK_Delete},
	{MWKEY_HOME, XK_Home},
	{MWKEY_END, XK_End},
	{MWKEY_TAB, XK_Tab},
	{MWKEY_BACKSPACE, XK_BackSpace},
	{MWKEY_ENTER, XK_Return},
	{MWKEY_ESCAPE, XK_Escape},
	{MWKEY_LCTRL, XK_Control_L},
	{MWKEY_RCTRL, XK_Control_R},
	{MWKEY_LSHIFT, XK_Shift_L},
	{MWKEY_RSHIFT, XK_Shift_R},
	{MWKEY_LALT, XK_Alt_L},
	{MWKEY_LALT, XK_Mode_switch},		/* MACOSX left or right alt/opt*/
	{MWKEY_RALT, XK_Alt_R},
	{MWKEY_LMETA, XK_Meta_L},
	{MWKEY_RMETA, XK_Meta_R},
	{MWKEY_PAUSE, XK_Pause},
	{MWKEY_PRINT, XK_Print},
	{MWKEY_SYSREQ, XK_Sys_Req},
	{MWKEY_BREAK, XK_Break},
	{MWKEY_NUMLOCK, XK_Num_Lock},
	{MWKEY_CAPSLOCK, XK_Caps_Lock},
	{MWKEY_SCROLLOCK, XK_Scroll_Lock},
	{MWKEY_F1, XK_F1},
	{MWKEY_F2, XK_F2},
	{MWKEY_F3, XK_F3},
	{MWKEY_F4, XK_F4},
	{MWKEY_F5, XK_F5},
	{MWKEY_F6, XK_F6},
	{MWKEY_F7, XK_F7},
	{MWKEY_F8, XK_F8},
	{MWKEY_F9, XK_F9},
	{MWKEY_F10, XK_F10},
	{MWKEY_F11, XK_F11},
	{MWKEY_F12, XK_F12},
	{MWKEY_KP1, XK_KP_End},
	{MWKEY_KP2, XK_KP_Down},
	{MWKEY_KP3, XK_KP_Page_Down},
	{MWKEY_KP4, XK_KP_Left},
	{MWKEY_KP5, XK_KP_Home},
	{MWKEY_KP6, XK_KP_Right},
	{MWKEY_KP7, XK_KP_Home},
	{MWKEY_KP8, XK_KP_Up},
	{MWKEY_KP9, XK_KP_Page_Up},
	{MWKEY_KP0, XK_KP_Insert},
	{MWKEY_KP_PERIOD, XK_KP_Delete},
	{MWKEY_KP_ENTER, XK_KP_Enter},
	{MWKEY_KP_DIVIDE, XK_KP_Divide},
	{MWKEY_KP_MULTIPLY, XK_KP_Multiply},
	{MWKEY_KP_MINUS, XK_KP_Subtract},
	{MWKEY_KP_PLUS, XK_KP_Add},
	{MWKEY_MENU, XK_Menu},
	{0xffff, 0xffff}
};


/* load Linux kernel keymap, ignores parameter*/
int 
XRefreshKeyboardMapping(XMappingEvent* event)
{ 
	return 0;
}

/* translate keycode to KeySym, no control/shift processing*/
/* no international keyboard support */
#if NeedWidePrototypes
KeySym XKeycodeToKeysym(Display *dpy, unsigned int kc, int index)
#else
KeySym XKeycodeToKeysym(Display *dpy, KeyCode kc, int index)
#endif
{
	NANOX_NX11_VSF_GET_CONTEXT();
	//DPRINTF("XKeycodeToKeysym called\n");
	int	i;
	MWKEY	mwkey;

	if (kc > 127)
		return NoSymbol;

	/* first convert scancode to mwkey*/
	mwkey = TranslateScancode(kc, modstate);
	switch (mwkey) {
	case MWKEY_NUMLOCK:
		modstate ^= MWKMOD_NUM;	
		break;
	case MWKEY_CAPSLOCK:
		modstate ^= MWKMOD_CAPS;	
		break;
	}
	
	/* then possibly convert mwkey to X KeySym*/
	for (i=0; mwkey_to_xkey[i].nxKey != 0xffff; i++) {
		if (mwkey == mwkey_to_xkey[i].nxKey)
			return mwkey_to_xkey[i].xKey;
	}

	/* assume X KeySym is same as MWKEY value*/
	return mwkey;
}
/* translate keyvalue to KeySym, no control/shift processing*/

KeySym
XMWKeyToKeysym(Display *dpy, unsigned int kv, int index)
{
	int	i;
	MWKEY	mwkey;

	//if (kv > 127)
	//	return NoSymbol;

	//DPRINTF("XMWKeyToKeysym called - %X\n",(unsigned int)kv);

	mwkey = kv;

	/* then possibly convert mwkey to X KeySym*/
	for (i=0; mwkey_to_xkey[i].nxKey != 0xffff; i++) {
		if (mwkey == mwkey_to_xkey[i].nxKey)
			return mwkey_to_xkey[i].xKey;
	}

	/* assume X KeySym is same as MWKEY value*/
	return mwkey;
}

/* translate event->keycode/event->y_root into KeySym, no control/shift processing*/
KeySym
XLookupKeysym(XKeyEvent *event, int index)
{
	/* event->y_root set in NextEvent.c*/
	return XMWKeyToKeysym(event->display, (unsigned int) event->y_root, index);
}

/* translate event->keycode into *keysym, control/shift processing*/
int
XLookupString(XKeyEvent *event, char *buffer, int nbytes, KeySym *keysym,
	XComposeStatus *status)
{
	NANOX_NX11_VSF_GET_CONTEXT();
	KeySym k;

	modstate &= 0xffff ^ MWKMOD_SHIFT;
	modstate &= 0xffff ^ MWKMOD_CTRL;

	/* translate Control/Shift*/
	if ((event->state & ControlMask) /* && k < 256*/) {
		modstate |= MWKMOD_CTRL;
		//k &= 0x1f;
	} else if (event->state & ShiftMask)
		modstate |= MWKMOD_SHIFT;
	else if( event->state & Mod1Mask)
		modstate |= MWKMOD_ALTGR;

	k = XLookupKeysym(event, 0);

	//DPRINTF("XLookupString called - %X\n",(unsigned int)k);

	if (keysym)
		*keysym = k;
	buffer[0] = (char)k;

	if (k & 0xFF00) {	/* non-ASCII key, return 0 for cursor keys in FLTK */
		switch (k) {
		case XK_BackSpace:
		case XK_Tab:
		case XK_Linefeed:
		case XK_Return:
		case XK_Escape:
		case XK_Delete:
			break;
		default:
			return 0;
		}
	}
	return 1;
}

/* Freeking ugly! */
KeySym
XStringToKeysym(_Xconst char *string)
{
	int i;

	for (i=0; i < NX_KEYSYMSTR_COUNT; i++)
		if (strcmp(nxKeyStrings[i].str, string) == 0)
			return nxKeyStrings[i].keysym;

	return NoSymbol;
}

char *XKeysymToString(KeySym ks)
{
	int i;

	DPRINTF("XKeysymToString called [%x]\n", (int)ks);
	for (i=0; i < NX_KEYSYMSTR_COUNT; i++)
		if (nxKeyStrings[i].keysym == ks) return nxKeyStrings[i].str;

	return NULL;
}

/* translate KeySym to KeyCode*/
KeyCode
XKeysymToKeycode(Display *dpy, KeySym ks)
{
	int i;

	for (i=0; i<128; ++i)
		if (kbd_keymap[i] == ks)
			return i;
	return NoSymbol;
}

/* Translate the keysym to upper case and lower case */

void
XConvertCase(KeySym in, KeySym *upper, KeySym *lower)
{
	if (in & MWKEY_NONASCII_MASK) 
		*upper = *lower = in;
	else {
		*upper = (in >= 'a' && in <= 'z')? in-'a'+'A': in;
		*lower = (in >= 'A' && in <= 'A')? in-'A'+'a': in;
	}
}
  
#if 0000
/*
 * Microwindows ttyscan.c compatible scancode conversion
 * table.  Note this is NOT the same as the Linux kernel
 * table due to the HACK XXX in ttyscan.c after getting
 * the kernel scancode.  FIXME
 */
#define UNKNOWN	0
static MWKEY mwscan_to_mwkey[128] = {
	UNKNOWN,	/*  0*/
	UNKNOWN,	/*  1*/
	UNKNOWN,	/*  2*/
	UNKNOWN,	/*  3*/
	UNKNOWN,	/*  4*/
	UNKNOWN,	/*  5*/
	UNKNOWN,	/*  6*/
	UNKNOWN,	/*  7*/
	UNKNOWN,	/*  8*/
	MWKEY_ESCAPE,	/*  9*/
	'1',		/* 10*/
	'2',		/* 11*/
	'3',		/* 12*/
	'4',		/* 13*/
	'5',		/* 14*/
	'6',		/* 15*/
	'7',		/* 16*/
	'8',		/* 17*/
	'9',		/* 18*/
	'0',		/* 19*/
	'-',		/* 20*/
	UNKNOWN,	/* 21*/
	MWKEY_BACKSPACE,/* 22*/
	MWKEY_TAB,	/* 23*/
	'q',		/* 24*/
	'w',		/* 25*/
	'e',		/* 26*/
	'r',		/* 27*/
	't',		/* 28*/
	'y',		/* 29*/
	'u',		/* 30*/
	'i',		/* 31*/
	'o',		/* 32*/
	'p',		/* 33*/
	'[',		/* 34*/
	']',		/* 35*/
	MWKEY_ENTER,	/* 36*/
	MWKEY_LCTRL,	/* 37*/
	'a',		/* 38*/
	's',		/* 39*/
	'd',		/* 40*/
	'f',		/* 41*/
	'g',		/* 42*/
	'h',		/* 43*/
	'j',		/* 44*/
	'k',		/* 45*/
	'l',		/* 46*/
	';',		/* 47*/
	'\'',		/* 48*/
	'`',		/* 49*/
	MWKEY_LSHIFT,	/* 50*/
	'\\',		/* 51*/
	'z',		/* 52*/
	'x',		/* 53*/
	'c',		/* 54*/
	'v',		/* 55*/
	'b',		/* 56*/
	'n',		/* 57*/
	'm',		/* 58*/
	',',		/* 59*/
	'.',		/* 60*/
	'/',		/* 61*/
	MWKEY_RSHIFT,	/* 62*/
	MWKEY_KP_MULTIPLY,/* 63*/
	MWKEY_LALT,	/* 64*/
	' ',		/* 65*/
	UNKNOWN,	/* 66*/
	MWKEY_F1,	/* 67*/
	MWKEY_F2,	/* 68*/
	MWKEY_F3,	/* 69*/
	MWKEY_F4,	/* 70*/
	MWKEY_F5,	/* 71*/
	MWKEY_F6,	/* 72*/
	MWKEY_F7,	/* 73*/
	MWKEY_F8,	/* 74*/
	MWKEY_F9,	/* 75*/
	MWKEY_F10,	/* 76*/
	UNKNOWN,	/* 77*/
	UNKNOWN,	/* 78*/
	MWKEY_KP7,	/* 79*/
	MWKEY_KP8,	/* 80*/
	MWKEY_KP9,	/* 81*/
	MWKEY_KP_MINUS,	/* 82*/
	MWKEY_KP4,	/* 83*/
	MWKEY_KP5,	/* 84*/
	MWKEY_KP6,	/* 85*/
	MWKEY_KP_PLUS,	/* 86*/
	MWKEY_KP1,	/* 87*/
	MWKEY_KP2,	/* 88*/
	MWKEY_KP3,	/* 89*/
	MWKEY_KP0,	/* 90*/
	MWKEY_KP_PERIOD,/* 91*/
	UNKNOWN,	/* 92*/
	UNKNOWN,	/* 93*/
	UNKNOWN,	/* 94*/
	MWKEY_F11,	/* 95*/
	MWKEY_F12,	/* 96*/
	MWKEY_HOME,	/* 97*/
	MWKEY_UP,	/* 98*/
	MWKEY_PAGEUP,	/* 99*/
	MWKEY_LEFT,	/*100*/
	UNKNOWN,	/*101*/
	MWKEY_RIGHT,	/*102*/
	MWKEY_END,	/*103*/
	MWKEY_DOWN,	/*104*/
	MWKEY_PAGEDOWN,	/*105*/
	MWKEY_INSERT,	/*106*/
	MWKEY_DELETE,	/*107*/
	MWKEY_KP_ENTER,	/*108*/
	MWKEY_RCTRL,	/*109*/
	UNKNOWN,	/*110*/
	MWKEY_PRINT,	/*111*/
	MWKEY_KP_DIVIDE, /*112*/
	MWKEY_RALT,	/*113*/
	UNKNOWN,	/*114*/
	UNKNOWN,	/*115*/
	UNKNOWN,	/*116*/
	UNKNOWN,	/*117*/
	UNKNOWN,	/*118*/
	UNKNOWN,	/*119*/
	UNKNOWN,	/*120*/
	UNKNOWN,	/*121*/
	UNKNOWN,	/*122*/
	UNKNOWN,	/*123*/
	UNKNOWN,	/*124*/
	UNKNOWN,	/*125*/
	UNKNOWN,	/*126*/
	UNKNOWN		/*127*/
};
#endif
