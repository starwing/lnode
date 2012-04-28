#ifndef lselem_h
#define lselem_h

#define LS_ATTRS(X) \
    X("clip",              CLIP              ) \
    X("clip-path",         CLIP_PATH         ) \
    X("color",             COLOR             ) \
    X("cx",                CX                ) \
    X("cy",                CY                ) \
    X("d",                 DATA              ) \
    X("dx",                DX                ) \
    X("dy",                DY                ) \
    X("fill",              FILL              ) \
    X("fill-opacity",      FILL_OPACITY      ) \
    X("font-size",         FONT_SIZE         ) \
    X("height",            HEIGHT            ) \
    X("id",                ID                ) \
    X("onabort",           ON_ABORT          ) \
    X("onactivate",        ON_ACTIVATE       ) \
    X("onbegin",           ON_BEGIN          ) \
    X("onclick",           ON_CLICK          ) \
    X("onend",             ON_END            ) \
    X("onerror",           ON_ERROR          ) \
    X("onfocusin",         ON_FOCUSIN        ) \
    X("onfocusout",        ON_FOCUSOUT       ) \
    X("onload",            ON_LOAD           ) \
    X("onmousedown",       ON_MOUSEDOWN      ) \
    X("onmousemove",       ON_MOUSEMOVE      ) \
    X("onmouseout",        ON_MOUSEOUT       ) \
    X("onmouseover",       ON_MOUSEOVER      ) \
    X("onmouseup",         ON_MOUSEUP        ) \
    X("onrepeat",          ON_REPEAT         ) \
    X("onresize",          ON_RESIZE         ) \
    X("onscroll",          ON_SCROLL         ) \
    X("onunload",          ON_UNLOAD         ) \
    X("onzoom",            ON_ZOOM           ) \
    X("opacity",           OPACITY           ) \
    X("pathLength",        PATH_LENGTH       ) \
    X("points",            POINTS            ) \
    X("r",                 R                 ) \
    X("rotate",            ROTATE            ) \
    X("rx",                RX                ) \
    X("ry",                RY                ) \
    X("spacing",           SPACING           ) \
    X("startOffset",       START_OFFSET      ) \
    X("stroke",            STROKE            ) \
    X("stroke-dasharray",  STROKE_DASHARRAY  ) \
    X("stroke-dashoffset", STROKE_DASHOFFSET ) \
    X("stroke-linecap",    STROKE_LINECAP    ) \
    X("stroke-linejoin",   STROKE_LINEJOIN   ) \
    X("stroke-opacity",    STROKE_OPACITY    ) \
    X("style",             STYLE             ) \
    X("visibility",        VISIBILITY        ) \
    X("width",             WIDTH             ) \
    X("x",                 X                 ) \
    X("x1",                X1                ) \
    X("x2",                X2                ) \
    X("xlink:href",        HREF              ) \
    X("y",                 Y                 ) \
    X("y1",                Y1                ) \
    X("y2",                Y2                ) \


enum ls_Attr {
#define X(a,b) LSA_##b,
    LS_ATTRS(X)
#undef X
    LS_ATTR_NUM
};

const char *ls_attr_string(int attr);
int ls_attr_from_string(const char *s);

int ls_attr_isevent(int attr);
const char *ls_attr_event_string(int attr);
int ls_attr_event_fromstring(const char *s);


#endif /* lselem_h */
