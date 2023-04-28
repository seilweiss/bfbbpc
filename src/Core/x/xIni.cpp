#include "xIni.h"

#include "xString.h"

#include <string>
#include <rwcore.h>

static char* TrimWhitespace(char* str)
{
    while (*str == ' ' || *str == '\t') str++;
    if (!*str) return str;
    
    char* c = str + (strlen(str) - 1);
    while (*c == ' ' || *c == '\t') c--;
    *(c+1) = '\0';
    
    return str;
}

xIniFile* xIniParse(char* buf, int len)
{
    int valueAlloc, sectionAlloc, i, ccr, clf, copen, lastCRLF;
    char* c, *tok, *val, *lastLine, *line, *ltoken;
    xIniFile* ini;
    
    ccr = 1;
    clf = 1;
    copen = 0;
    lastCRLF = -1;

    for (i = 0; i < len; i++) {
        switch (buf[i]) {
        case '\n':
            lastCRLF = i;
            clf++;
            break;
        case '\r':
            lastCRLF = i;
            ccr++;
            break;
        case '[':
            copen++;
            break;
        }
    }

    if (clf > ccr) ccr = clf;

    sectionAlloc = copen;
    valueAlloc = ccr;

    ini = (xIniFile*)RwMalloc(sizeof(xIniFile) +
                              valueAlloc * sizeof(xIniValue) +
                              sectionAlloc * sizeof(xIniSection) +
                              (len - lastCRLF));
    ini->mem = NULL;
    ini->NumValues = 0;
    ini->NumSections = 0;
    ini->Values = (xIniValue*)(ini + 1);
    ini->Sections = (xIniSection*)(ini->Values + valueAlloc);
    lastLine = (char*)(ini->Sections + sectionAlloc);

    strncpy(lastLine, buf + (lastCRLF + 1), len - (lastCRLF + 1));
    lastLine[len - (lastCRLF + 1)] = '\0';

    if (lastCRLF >= 0) buf[lastCRLF] = '\0';
    else buf[0] = '\0';

    line = xStrTok(buf, "\n\r", &ltoken);
    if (!line) {
        line = xStrTok(lastLine, "\n\r", &ltoken);
        lastLine = NULL;
    }

    while (line) {
        line = TrimWhitespace(line);
        if (*line != '#' && *line != '\0') {
            if (*line == '[') {
                if (c = std::strstr(line, "]")) {
                    *c = '\0';
                    c = TrimWhitespace(line + 1);
                    if (*c != '\0') {
                        ini->Sections[ini->NumSections].sec = c;
                        ini->Sections[ini->NumSections].first = ini->NumValues;
                        ini->Sections[ini->NumSections].count = 0;
                        ini->NumSections++;
                    }
                }
            } else {
                if (c = std::strstr(line, "=")) {
                    *c = '\0';
                    tok = TrimWhitespace(line);
                    if (*tok != '\0') {
                        line = c + 1;
                        if (c = std::strstr(line, "#")) {
                            *c = '\0';
                        }
                        val = TrimWhitespace(line);
                        ini->Values[ini->NumValues].tok = tok;
                        ini->Values[ini->NumValues].val = val;
                        ini->NumValues++;
                        if (ini->NumSections) {
                            ini->Sections[ini->NumSections-1].count++;
                        }
                    }
                }
            }
        }
        line = xStrTok(NULL, "\n\r", &ltoken);
        if (!line && lastLine) {
            line = xStrTok(lastLine, "\n\r", &ltoken);
            lastLine = NULL;
        }
    }

    return ini;
}

void xIniDestroy(xIniFile* ini)
{
    RwFree(ini->mem);
    RwFree(ini);
}

int xIniGetIndex(xIniFile* ini, char* tok)
{
    for (int i = 0; i < ini->NumValues; i++) {
        if (!xStricmp(ini->Values[i].tok, tok)) {
            return i;
        }
    }
    return -1;
}

int xIniGetInt(xIniFile* ini, char* tok, int def)
{
    int index = xIniGetIndex(ini, tok);
    if (index == -1) {
        return def;
    }
    return atoi(ini->Values[index].val);
}

float xIniGetFloat(xIniFile* ini, char* tok, float def)
{
    int index = xIniGetIndex(ini, tok);
    if (index == -1) {
        return def;
    }
    return (float)atof(ini->Values[index].val);
}

char* xIniGetString(xIniFile* ini, char* tok, char* def)
{
    int index = xIniGetIndex(ini, tok);
    if (index == -1) {
        return def;
    }
    return ini->Values[index].val;
}