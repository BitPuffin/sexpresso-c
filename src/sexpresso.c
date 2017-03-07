/* Author: Isak Andersson 2017 bitpuffin dot com */

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sexpresso.h>

enum { false, true };

sexpresso_sexp sexpressoCreateString(char const* Str) {
	return sexpressoCreateStringUnescapedMove(sexpressoEscape(Str));
}

sexpresso_sexp sexpressoCreateStringUnescaped(char const* Str) {
	size_t Length = strlen(Str) + 1;
	char* Val = malloc(sizeof(char)*Length);
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
		Dest->Value.Sexp.Sexps = malloc(sizeof(sexpresso_sexp) * Sexp->Value.Sexp.Count);
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

static const size_t EscapeCharCount = 11;
static char const* const EscapeChars = "n\"'\\ftrvba?";
static char const* const EscapeVals  = "\n\"'\\\f\t\r\v\b\a\?";
/* static const char* EscapeChars = { 'n',  '"', '\'', '\\',  'f',  't',  'r',  'v',  'b',  'a',  '?' }; */
/* static const char* EscapeVals  = { '\n', '"', '\'', '\\', '\f', '\t', '\r', '\v', '\b', '\a', '\?' }; */

struct string_build {
	size_t Cap;
	size_t Count;
	char* Str;
};
static const size_t INITIAL_STRING_CAP   = 1024;
static const float  STRING_GROWTH_FACTOR = 1.5f;

static void grow(struct string_build* Build) {
	Build->Cap *= STRING_GROWTH_FACTOR;
	Build->Str = realloc(Build->Str, sizeof(char)*Build->Cap);
}

static void insertChar(struct string_build* Build, char c) {
	if(Build->Count == Build->Cap) grow(Build);
	Build->Str[Build->Count] = c;
	++Build->Count;
}

static void insertStringValue(struct string_build* Build, const char* Str) {
	size_t Len = strlen(Str);
	char const* c;
	if(0 == Len) { insertChar(Build, '"'); insertChar(Build, '"'); return; }
	for(c=Str; c < Str + Len; ++c) if(' ' == *c) break;
	if(c == Str + Len) {
		if((Build->Count + Len) >= Build->Cap) grow(Build);
		strncpy(Build->Str + Build->Count, Str, Len);
		Build->Count += Len;
	} else {
		if((Build->Count + Len + 2) >= Build->Cap) grow(Build);
		insertChar(Build, '"');
		for(c=Str; c < Str + Len; ++c) {
			char const* esc;
			for(esc=EscapeVals; esc < EscapeVals + EscapeCharCount; ++esc) if(*esc == *c) break;
			if(esc == EscapeVals + EscapeCharCount) insertChar(Build, *c);
			else { insertChar(Build, '\\'); insertChar(Build, *(EscapeChars + (esc - EscapeVals))); }
		}
		insertChar(Build, '"');
	}
}

static void toStringImpl(struct string_build* Build, sexpresso_sexp* Sexp) {
	switch(Sexp->Kind) {
	case SEXPRESSO_STRING:
		insertStringValue(Build, Sexp->Value.Str);
		break;
	case SEXPRESSO_SEXP:
		insertChar(Build, '(');
		switch(Sexp->Value.Sexp.Count) {
		case 0:
			break;
		case 1:
			toStringImpl(Build, &Sexp->Value.Sexp.Sexps[0]);
			break;
		default:{
			sexpresso_sexp* i;
			for(i=Sexp->Value.Sexp.Sexps; i < Sexp->Value.Sexp.Sexps + Sexp->Value.Sexp.Count; ++i) {
				toStringImpl(Build, i);
				if(i != Sexp->Value.Sexp.Sexps + Sexp->Value.Sexp.Count - 1) insertChar(Build, ' ');
			}
		}
		}
		insertChar(Build, ')');
	}
}

char const* sexpressoToString(sexpresso_sexp const* Sexp) {
	struct string_build Build;
	Build.Cap = INITIAL_STRING_CAP;
	Build.Count = 0;
	Build.Str = malloc(sizeof(char) * Build.Cap);

	switch(Sexp->Kind) {
	case SEXPRESSO_STRING:
		insertStringValue(&Build, Sexp->Value.Str);
		break;
	case SEXPRESSO_SEXP: {
		sexpresso_sexp* i;
		for(i=Sexp->Value.Sexp.Sexps; i<Sexp->Value.Sexp.Sexps + Sexp->Value.Sexp.Count; ++i) {
			toStringImpl(&Build, i);
			if(i != Sexp->Value.Sexp.Sexps + Sexp->Value.Sexp.Count - 1) insertChar(&Build, ' ');
		}
		break;
	}
	}

	Build.Str = realloc(Build.Str, sizeof(char)*Build.Count + 1);
	Build.Str[Build.Count] = '\0';
	return Build.Str;
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

void sexpressoDestroy(sexpresso_sexp* Sexp) {
}

struct stack {
	sexpresso_sexp Sexp;
	struct stack* Prev;
};

int sexpressoParse(sexpresso_sexp* Dest, char const* Str, sexpresso_error* Err) {
	struct stack* Stack = calloc(1, sizeof(struct stack));
	struct stack* TmpStack = NULL;

	char const* It;
	char const* NextIt = Str;
	for(It=Str; *It!='\0'; It=NextIt) {
		++NextIt;
		if(isspace(*It)) continue;
		switch(*It) {
		case '(':
			TmpStack = Stack;
			Stack = calloc(1, sizeof(struct stack));
			Stack->Prev = TmpStack;
			break;
		case ')':
			TmpStack = Stack;
			Stack = Stack->Prev;
			if(Stack == NULL) {
				if(Err != NULL) {
					Err->Code = SEXPRESSO_ERROR_EXCESS_CLOSING_PARENTHESES;
				}
				return 1;
			}
			sexpressoAddChildMove(&Stack->Sexp, &TmpStack->Sexp);
			break;
		case '"': {
			char* ResultStr;
			const char* Iter;
			const char* i = It+1;
			const char* Start = i;
			size_t EscapeCount = 0;
			size_t StrSize;
			for(; *i!='\0'; ++i) {
				if(*i == '\\') { ++EscapeCount; ++i; continue; } /* @FIXME: Someone could sneak in a newline after a backslash and get away with it */
				if(*i == '"') break;
				if(*i == '\n') {
					if(Err != NULL) {
						Err->Code = SEXPRESSO_ERROR_UNEXPECTED_NEWLINE_IN_STRING_LITERAL;
					}
					return 1;
				}
			}
			if('\0' == i) {
				if(Err != NULL) {
					Err->Code = SEXPRESSO_ERROR_UNTERMINATED_STRING_LITERAL;
				}
				return 1;
			}
			StrSize = i - Start - EscapeCount + 1;
			ResultStr = malloc(sizeof(char)*StrSize);
			for(Iter=Start; Iter != i; ++Iter) {
				switch(*Iter) {
				case '\\': {
					++Iter;
					if(Iter == i) {
						if(Err != NULL) {
							Err->Code = SEXPRESSO_ERROR_UNFINISHED_ESCAPE_SEQUENCE_AT_END_OF_STRING;
						}
						return 1;
					}
					{
						size_t pos;
						for(pos=0; pos<EscapeCharCount; ++pos) {
							if(EscapeChars[pos] == *Iter) break;
						}
						if(EscapeCharCount == pos) {
							if(Err != NULL) {
								Err->Code = SEXPRESSO_ERROR_INVALID_ESCAPE_CHARACTER;
							}
							return 1;
						}
						ResultStr[Start - Iter] = EscapeVals[pos];
						break;
					}
				}
				default:
					ResultStr[Start - Iter] = *Iter;
				}
			}
			ResultStr[StrSize-1] = '\0';
			sexpressoAddChildStringUnescapedMove(&Stack->Sexp, ResultStr);
			NextIt = i + 1;
			break;
			}
		case ';':
			for(; *NextIt != '\0' && *NextIt != '\n' && *NextIt != '\r'; ++NextIt) {}
			for(; *NextIt != '\0' && (*NextIt == '\n' || *NextIt == '\r'); ++NextIt) {}
			break;
		default: {
			const char* SymEnd;
			char* Sym;
			size_t SymSize;
			for(SymEnd = It; *SymEnd != '\0' && !isspace(*SymEnd) && *SymEnd != ')'; ++SymEnd) {}
			SymSize = It - SymEnd + 1; /* @XXX: I suspect this math is wrong, needs testing */
			Sym = malloc(sizeof(char)*SymSize);
			strncpy(Sym, It, SymSize - 2);
			Sym[SymSize-1] = '\0';
			sexpressoAddChildStringUnescapedMove(&Stack->Sexp, Sym);
			NextIt = SymEnd;
		}
		}
	}
	if(Stack->Prev != NULL) {
		if(Err != NULL) {
			Err->Code = SEXPRESSO_ERROR_INCOMPLETE_SEXP;
		}
		return 1;
	}

	*Dest = Stack->Sexp;

	return 0;
}

const char* sexpressoEscape(char const* str) {
	return NULL;
}
