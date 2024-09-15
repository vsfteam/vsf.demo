#ifndef __NANOX_NX11_VSF__
#define __NANOX_NX11_VSF__

#include "shell/sys/linux/vsf_linux.h"

#include "X11/X.h"
#include "X11/Xatom.h"
#include "X11/Xutil.h"
#include "X11/Xresource.h"

#include "mwtypes.h"

// Atom.c

#define SZHASHTABLE     64

struct hash_t {
    char *name;
    Atom atom;
    struct hash_t *next;
};

// ChProperty.c

#define SZHASHTABLE	32

struct window_props {
	Atom property;
	Atom type;
	int format;
	unsigned char *data;
	int nelements;
	int bytes;
	struct window_props *next;
};

struct windows {
	Window w;
	struct window_props *properties;
	struct windows *next;
};

// Context.c

typedef struct _XCList {
	Display *display;
	XID rid;
	XContext context;
	XPointer data;

	struct _XCList	*prev;
	struct _XCList	*next;
} XCList;

// Extension.c

#define SHAPENAME "SHAPE"
#define SHAPE_MAJOR_VERSION	1	/* current version numbers */
#define SHAPE_MINOR_VERSION	1

typedef struct _EXT_Manage {
	char *name;
	char ver;
	int event;
	int error;
} EXT_Manage;

// Selection.c

struct sstruct {
	Atom selection;
	Window owner;
	Time time;
	struct sstruct *next;
};

// font_find.c

struct _list {
	char **list;
	int alloc;
	int used;
	struct _list *next;
};

// Quarks.c

typedef unsigned long Entry;

// Common

#define global_vsf_nanox_nx11_ctx       ((vsf_nanox_nx11_ctx_t *)(vsf_linux_pls_get(vsf_nanox_nx11_pls_idx)->data))
#define _nxCursorFont		(global_vsf_nanox_nx11_ctx->opendis.__nxCursorFont)
#define _Xdebug				(global_vsf_nanox_nx11_ctx->opendis.__Xdebug)
#define _XLockMutex_fn		(global_vsf_nanox_nx11_ctx->opendis.__XLockMutex_fn)
#define _XUnlockMutex_fn	(global_vsf_nanox_nx11_ctx->opendis.__XUnlockMutex_fn)
#define _Xglobal_lock		(global_vsf_nanox_nx11_ctx->opendis.__Xglobal_lock)

typedef struct vsf_nanox_nx11_ctx_t {
    struct {
        struct hash_t *_hash_list[SZHASHTABLE];
        unsigned long _atom_id;
    } atom;
    struct {
        struct windows *_window_list[SZHASHTABLE];
    } chproperty;
    struct {
        nxColormap *_colormap_hash[32];
        int _colormap_id;
        Colormap __defaultColormap;
    } colormap;
    struct {
        XCList _xcl;
    } context;
    struct {
        XIOErrorHandler __ioerrorfunc;
        XErrorHandler __errorfunc;
    } errorhandler;
    struct {
        EXT_Manage _exmanage[2];
        int _ext;
    } extension;
    struct {
        struct {
            XColor _fg;
            XColor _bg;
        } XCreateFontCursor;
    } fontcursor;
    struct {
        struct {
            GR_TIMEOUT _lasttime;
        } translateNXEvent;
        XEvent _saved_event;
        int _saved;
    } nextevent;
    struct {
        struct sstruct *_select_list;
    } selection;
    struct {
        Font __nxCursorFont;
        int __Xdebug;
        int (*__XLockMutex_fn)();
        int (*__XUnlockMutex_fn)();
        int *__Xglobal_lock;
        struct {
            int _fd;
        } XOpenDisplay;
    } opendis;
    struct {
        char **__nxfontlist;
        int __nxfontcount;
        struct _list *_g_fontlist;
    } font_find;
    struct {
        XrmQuark _nextQuark;
        unsigned long _quarkMask;
        Entry _zero;
        Entry *_quarkTable;
        unsigned long _quarkRehash;
        XrmString **_stringTable;
#ifdef PERMQ
        Bits **_permTable;
#endif
        XrmQuark _nextUniq;
        char *_neverFreeTable;
        int _neverFreeTableSize;
    } quarks;
    struct {
        XrmQuark _XrmQString;
        XrmQuark _XrmQANY;
        unsigned char *_resourceQuarks;
        XrmQuark _maxResourceQuark;
    } xrm;
    struct {
        MWKEYMOD _modstate
    } strkeysym;
} vsf_nanox_nx11_ctx_t;

extern int vsf_nanox_nx11_pls_idx;
#ifdef PERMQ
#define NANOX_NX11_VSF_INIT_PROCESS_DATA_PERMQ(dptr)                            \
    {                                                                           \
        dptr->quarks._permTable = -1;                                           \
    }
#else
#define NANOX_NX11_VSF_INIT_PROCESS_DATA_PERMQ(...)
#endif
#define NANOX_NX11_VSF_INIT_PROCESS_DATA()                                      \
    {                                                                           \
        if (vsf_nanox_nx11_pls_idx < 0) {                                       \
            vsf_nanox_nx11_pls_idx = vsf_linux_pls_alloc();                     \
            VSF_LINUX_ASSERT(vsf_nanox_nx11_pls_idx >= 0);                      \
        }                                                                       \
        vsf_linux_localstorage_t *pls = vsf_linux_pls_get(vsf_nanox_nx11_pls_idx);\
        VSF_LINUX_ASSERT(pls != NULL);											\
        if (NULL == pls->data) {                                                \
            vsf_nanox_nx11_ctx_t *dptr = malloc(sizeof(vsf_nanox_nx11_ctx_t));  \
            VSF_LINUX_ASSERT(dptr != NULL);                                     \
            pls->data = dptr;                                                   \
            memset(dptr, 0, sizeof(vsf_nanox_nx11_ctx_t));                      \
            dptr->atom._atom_id = XA_LAST_PREDEFINED + 32;                      \
            dptr->colormap._colormap_id = -1;                                   \
            dptr->extension._exmanage[0] = (EXT_Manage){ SHAPENAME, SHAPE_MAJOR_VERSION, LASTEvent+1, 0, };\
	        dptr->extension._exmanage[1] = (EXT_Manage){ NULL, 0, 0, 0 };       \
            dptr->fontcursor.XCreateFontCursor._bg = (XColor){ 0, 65535, 65535, 65535 };\
            dptr->opendis.XOpenDisplay._fd = -1;                                \
            dptr->quarks._nextQuark = 1;                                        \
            dptr->quarks._quarkTable = &dptr->quarks._zero;                     \
            dptr->xrm._maxResourceQuark = -1;                                   \
            NANOX_NX11_VSF_INIT_PROCESS_DATA_PERMQ(dptr);                       \
        }                                                                       \
    }

#define NANOX_NX11_VSF_GET_CONTEXT()                                            \
    vsf_nanox_nx11_ctx_t *vsf_nanox_nx11_ctx = (vsf_nanox_nx11_ctx_t*)          \
        vsf_linux_pls_get(vsf_nanox_nx11_pls_idx)->data;

#endif