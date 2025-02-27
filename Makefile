# $NetBSD: Makefile,v 1.2 2021/05/08 14:11:37 cjep Exp $

PROG=	audiodisp
SRCS+=	audio_intensity.c audio_ctrl.c audio_stream.c audio_displays.c

LDADD+=	-lcurses -lm
DPADD+=	${LIBCURSES} ${LIBM}

WARNS=	6

.include <bsd.prog.mk>
