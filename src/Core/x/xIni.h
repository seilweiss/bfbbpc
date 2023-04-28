#pragma once

struct xIniValue
{
    char* tok;
    char* val;
};

struct xIniSection
{
    char* sec;
    int first;
    int count;
};

struct xIniFile
{
    int NumValues;
    int NumSections;
    xIniValue* Values;
    xIniSection* Sections;
    void* mem;
    char name[256];
    char pathname[256];
};

xIniFile* xIniParse(char* buf, int len);
void xIniDestroy(xIniFile* ini);
int xIniGetIndex(xIniFile* ini, char* tok);
int xIniGetInt(xIniFile* ini, char* tok, int def);
float xIniGetFloat(xIniFile* ini, char* tok, float def);
char* xIniGetString(xIniFile* ini, char* tok, char* def);