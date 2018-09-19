#ifndef NATIVE_OS_H
#define NATIVE_OS_H

typedef struct utox_save UTOX_SAVE;

void openurl(char *str);

// Linux only.
void setselection(char *data, uint16_t length);

void config_osdefaults(UTOX_SAVE *r);

#endif
