#ifndef KEYLOG_H
#define KEYLOG_H

// Read from keyboard until the keycode 'end' is read
ssize_t dump_raw(int kbd, int log, int end, int ts_interval);
ssize_t dump_ascii(int kbd, int log, int end, int ts_interval);
ssize_t dump_log(int kbd, int log, int end, int ts_interval);
#endif
