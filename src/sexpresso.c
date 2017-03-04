/* Author: Isak Andersson 2017 bitpuffin dot com */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <sexpresso.h>

enum { false, true };

sexpresso_sexp sexpressoCreateString(char const* Str) {
	return sexpressoCreateStringUnescapedMove(sexpressoEscape(Str));
}

sexpresso_sexp sexpressoCreateStringUnescaped(char const* Str) {
	size_t Length = strlen(Str) + 1;
	char* Val = (char*)malloc(sizeof(char)*Length);
	strncpy(Val, Str, Length);
	return sexpressoCreateStringUnescapedMove(Val);
}

sexpresso_sexp sexpressoCreateStringUnescapedMove(char const* Str) {
	sexpresso_sexp Expr;
	Expr.Kind = SEXPRESSO_STRING;
	Expr.Value.Str = Str;
	return Expr;
}

static void copySexp(sexpresso_sexp* Dest, sexpresso_sexp* Sexp) {
	switch(Sexp->Kind) {
	case SEXPRESSO_SEXP:
		Dest->Kind = SEXPRESSO_SEXP;
		Dest->Value.Sexp.Count = Sexp->Value.Sexp.Count;
		Dest->Value.Sexp.Sexps = (sexpresso_sexp*)malloc(sizeof(sexpresso_sexp) * Sexp->Value.Sexp.Count);
		{
			size_t i;
			for(i=0; i<Sexp->Value.Sexp.Count; ++i)
				copySexp(Dest->Value.Sexp.Sexps+i, Sexp->Value.Sexp.Sexps+i);
		}
		break;
	case SEXPRESSO_STRING:
		*Dest = sexpressoCreateStringUnescaped(Sexp->Value.Str);
	}
}

void sexpressoAddChild(sexpresso_sexp* Dest, sexpresso_sexp* Child) {
	sexpresso_sexp NewSexp;
	copySexp(&NewSexp, Child);
	sexpressoAddChildMove(Dest, &NewSexp);
}

static void pushBackMove(sexpresso_sexp* Dest, sexpresso_sexp* Value) {
	size_t Last = Dest->Value.Sexp.Count++; /* Get the value and increment AFTER */
	Dest->Value.Sexp.Sexps = realloc(Dest->Value.Sexp.Sexps, sizeof(sexpresso_sexp) * Dest->Value.Sexp.Count);
	Dest->Value.Sexp.Sexps[Last] = *Value;
}

void sexpressoAddChildMove(sexpresso_sexp* Dest, sexpresso_sexp* Child) {
	if(Dest->Kind == SEXPRESSO_STRING) {
		char const* str = Dest->Value.Str;
		Dest->Kind = SEXPRESSO_SEXP;
		Dest->Value.Sexp.Sexps = malloc(sizeof(sexpresso_sexp) * 2);
		Dest->Value.Sexp.Sexps[0] = sexpressoCreateStringUnescapedMove(str);
		Dest->Value.Sexp.Sexps[1] = *Child;
		Dest->Value.Sexp.Count = 2;
	} else {
		pushBackMove(Dest, Child);
	}
}

void sexpressoAddChildString(sexpresso_sexp* Dest, char const* Str) {
	sexpresso_sexp StrExp = sexpressoCreateString(Str);
	sexpressoAddChildMove(Dest, &StrExp);
}

void sexpressoAddChildStringUnescaped(sexpresso_sexp* Dest, char const* Str) {
	sexpresso_sexp StrExp = sexpressoCreateStringUnescaped(Str);
	sexpressoAddChildMove(Dest, &StrExp);
}

void sexpressoAddChildStringUnescapedMove(sexpresso_sexp* Dest, char const* Str) {
	sexpresso_sexp StrExp = sexpressoCreateStringUnescapedMove(Str);
	sexpressoAddChildMove(Dest, &StrExp);
}

size_t sexpressoChildCount(sexpresso_sexp const* Sexp) {
	switch(Sexp->Kind) {
	case SEXPRESSO_SEXP:
		return Sexp->Value.Sexp.Count;
	case SEXPRESSO_STRING:
		return 1;
	}
}

int sexpressoIsString(sexpresso_sexp const* Sexp) {
	return Sexp->Kind == SEXPRESSO_STRING;
}

int sexpressoIsSexp(sexpresso_sexp const* Sexp) {
	return Sexp->Kind == SEXPRESSO_SEXP;
}

int sexpressoIsNil(sexpresso_sexp const* Sexp) {
	return Sexp->Kind == SEXPRESSO_SEXP && Sexp->Value.Sexp.Count == 0;
}

struct sexp_stack {
	sexpresso_sexp Sexp;
	struct sexp_stack* Prev;
};

static const size_t EscapeCharCount = 11;
static const char* EscapeChars = "n\"'\\ftrvba?";
static const char* EscapeVals  = "\n\"'\\\f\t\r\v\b\a\?";
/* static const char* EscapeChars = { 'n',  '"', '\'', '\\',  'f',  't',  'r',  'v',  'b',  'a',  '?' }; */
/* static const char* EscapeVals  = { '\n', '"', '\'', '\\', '\f', '\t', '\r', '\v', '\b', '\a', '\?' }; */

sexpresso_sexp* sexpressoParse(char const* str, char const** error) {
	return NULL;
}

const char* sexpressoEscape(char const* str) {
	return NULL;
}
