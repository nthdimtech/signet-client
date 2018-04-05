#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <fcntl.h>
#include <QApplication>

extern "C" {

#include "signetdev/host/signetdev_unix.h"
#include "signetdev/host/signetdev_priv.h"

#include <stdlib.h>

JNIEXPORT void JNICALL Java_com_nthdimtech_SignetActivity_notifyDeviceAttached(JNIEnv* env, jobject *self, jint fd, jint inEp, jint outEp, jboolean hasKeyboard)
{
	Q_UNUSED(env);
	Q_UNUSED(self);
	fcntl(fd, F_SETFL, O_NONBLOCK);
	struct attach_message *m = (struct attach_message *)malloc(sizeof(struct attach_message));
	m->fd = fd;
	m->rx_endpoint = inEp;
	m->tx_endpoint = outEp;
	m->has_keyboard = hasKeyboard ? 1 : 0;
	signetdev_priv_issue_command_no_resp(SIGNETDEV_CMD_FD_ATTACHED, m);
}

JNIEXPORT void JNICALL Java_com_nthdimtech_SignetActivity_notifyDeviceDetached(JNIEnv* env, jobject *self, jint fd)
{
	Q_UNUSED(env);
	Q_UNUSED(self);
	signetdev_priv_issue_command_no_resp(SIGNETDEV_CMD_FD_DETACHED, (void *)((intptr_t)(fd)));
}

}
