#include "project.h"
#include "appweb.h"

#define FIFO_PATH	"/tmp/pduc2appweb"

const char   mainFrameDoctype[] =
	"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n\n""<html xmlns=\"http://www.w3.org/1999/xhtml\">";

char* escapeNumberSign(char *buf) {
	char   esc[1024];
	char   *p0, *p1;
	
	if (strlen(buf) > sizeof(esc)/2 - 1)   return NULL;
	memset(esc, 0, sizeof(esc));
	for (p0 = buf, p1 = esc; *p0; p0++, p1++) {
		*p1 = *p0;
		if (*p0 == '%') {
			++p1;
			*p1 = *p0;
		}
	}
	return esc;
}				
				
bool action_output_file(char *filepath, HttpQueue *q) {
	FILE   *fd;
	char   buf[1024];

	fd = fopen(filepath, "r+");
	if (!fd) {
		fd = fopen("/xprod/xweb/xfs/error.html", "r");
		if (!fd)   return 0;
	}
	memset(buf, '\0', sizeof(buf));
	int size;
	mprLog("critical http auth", 3, "size@");
	sleep(1);
	while ((size = fread(buf, 1, sizeof(buf)/2-1, fd)) > 0) {
		mprLog("critical http auth", 3, "size- %d", size);
		mprLog("critical http auth", 3, "out::: %s", buf);
		httpWrite(q, escapeNumberSign(buf));
		memset(buf, '\0', sizeof(buf));
	}
	mprLog("critical http auth", 3, "size-- %d", size);
	return 1;
}

PUBLIC void action_get(HttpConn *conn)
{
	HttpQueue   *q = conn->writeq;
	char   prm[128];
	char   *page = NULL;
	
	page = httpGetParam(conn, "p", NULL);
	if (page !=  NULL) 
		sprintf(prm, "username=%s,/%s",conn->username, page);
	else
		return;
	TICK ts, te;
	tick_run();
	ts = system_tick;
	int ret = xif_sets(app_pduc, pduc_Oid_webRequest, 0, prm);
	// tick_run();
	te = system_tick;
	mprLog("critical http auth", 3, "ts = %d, te = %d, ret = %d", ts, te, ret);

	if (ret == 0) {
		httpSetStatus(conn, 200);
		httpWrite(q, mainFrameDoctype);
		/*
			Add desired headers. "Set" will overwrite, add will create if not already defined.
		 */		
		httpAddHeaderString(conn, "Content-Type", "text/html");
		httpSetHeaderString(conn, "Cache-Control", "no-cache");
		mprLog("critical http auth", 3, "come--");
		if (!action_output_file(/*"/tmp/boa/result.html"*/FIFO_PATH, q)) {
			httpWrite(q, "file loss");
		}
		mprLog("critical http auth", 3, "--here");
	   /*
			Call finalize output and close the request.
			Delay closing if you want to do asynchronous output and close later.
		 */			
		httpFinalize(conn);
	} 
}