#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <sexpresso.h>

static void test_empty_string() {
	char const* Str = "";
	assert_true(strlen(Str) == 0);

	sexpresso_sexp Sexp;
	assert_false(sexpressoParse(&Sexp, Str, NULL));

	assert_int_equal(Sexp.Kind, SEXPRESSO_SEXP);
	assert_int_equal(sexpressoChildCount(&Sexp), 0);
	assert_true(sexpressoIsNil(&Sexp));
}

static void test_empty_sexp() {
	char const* Str = "()";

	sexpresso_sexp Sexp;
	assert_false(sexpressoParse(&Sexp, Str, NULL));

	assert_int_equal(Sexp.Kind, SEXPRESSO_SEXP);
	assert_int_equal(sexpressoChildCount(&Sexp), 1);
	assert_int_equal(Sexp.Value.Sexp.Sexps[0].Kind, SEXPRESSO_SEXP);
	assert_int_equal(sexpressoChildCount(&Sexp.Value.Sexp.Sexps[0]), 0);

	char const* Serialized = sexpressoToString(&Sexp);
	assert_string_equal(Serialized, Str);
	free((void*)Serialized);

	sexpressoDestroy(&Sexp);
}

static void test_multiple_empty_sexp() {
	char const* Str = "()\n() ()";

	sexpresso_sexp Sexp;
	assert_false(sexpressoParse(&Sexp, Str, NULL));

	size_t Count = sexpressoChildCount(&Sexp);
	assert_int_equal(Count, 3);

	for(sexpresso_sexp* i = Sexp.Value.Sexp.Sexps; i < Sexp.Value.Sexp.Sexps + Count; ++i) {
		assert_true(sexpressoIsNil(i));
	}

	char const* Serialized = sexpressoToString(&Sexp);
	assert_string_equal(Serialized, "() () ()");
	free((void*)Serialized);

	sexpressoDestroy(&Sexp);
}

static void equality() {
	char const* Str = "hi there (what a cool (little list) parser) (library)";

	sexpresso_sexp Sexp;
	assert_false(sexpressoParse(&Sexp, Str, NULL));

	sexpresso_sexp Outer = {0};
	sexpressoAddChildString(&Outer, "hi");
	sexpressoAddChildString(&Outer, "there");

	sexpresso_sexp What = {0};
	sexpressoAddChildString(&What, "what");
	sexpressoAddChildString(&What, "a");
	sexpressoAddChildString(&What, "cool");

	sexpresso_sexp Little = {0};
	sexpressoAddChildString(&Little, "little");
	sexpressoAddChildString(&Little, "list");
	sexpressoAddChildMove(&What, &Little);

	sexpressoAddChildString(&What, "parser");

	sexpressoAddChildMove(&Outer, &What);

	sexpresso_sexp Library = {0};
	sexpressoAddChildString(&Library, "library");

	sexpressoAddChildMove(&Outer, &Library);

	assert_true(sexpressoEqual(&Sexp, &Outer));
	char const* Serialized = sexpressoToString(&Outer);
	assert_int_equal(strcmp(Str, Serialized), 0);
	free((void*) Serialized);

	sexpressoDestroy(&Sexp);
	sexpressoDestroy(&Outer);
}

static void inequality() {
	sexpresso_sexp A, B = {0};

	assert_false(sexpressoParse(&A, "this (one is nothing)", NULL));
	assert_false(sexpressoParse(&B, "like the (other)", NULL));

	assert_false(sexpressoEqual(&A, &B));

	sexpressoDestroy(&A);
	sexpressoDestroy(&B);
}

static void string_literal() {
	sexpresso_sexp Sexp;
	assert_false(sexpressoParse(&Sexp, "\"hello world\" hehe", NULL));
	assert_int_equal(sexpressoChildCount(&Sexp), 2);
	assert_int_equal(Sexp.Value.Sexp.Sexps[0].Kind, SEXPRESSO_STRING);
	assert_int_equal(Sexp.Value.Sexp.Sexps[1].Kind, SEXPRESSO_STRING);
	assert_string_equal(Sexp.Value.Sexp.Sexps[0].Value.Str, "hello world");
	assert_string_equal(Sexp.Value.Sexp.Sexps[1].Value.Str, "hehe");
	sexpressoDestroy(&Sexp);
}

static void hierarchy_query() {
	sexpresso_sexp Sexp, Test;
	assert_false(sexpressoParse(&Sexp, "(myshit (a (name me) (age 2)) (b (name you) (age 1)))", NULL));

	assert_false(sexpressoParse(&Test, "name me", NULL));
	assert_true(sexpressoEqual(sexpressoGetChildByPath(&Sexp, "myshit/a/name"), &Test));
	sexpressoDestroy(&Test);

	assert_false(sexpressoParse(&Test, "age 2", NULL));
	assert_true(sexpressoEqual(sexpressoGetChildByPath(&Sexp, "myshit/a/age"), &Test));
	sexpressoDestroy(&Test);

	assert_false(sexpressoParse(&Test, "a (name me) (age 2)", NULL));
	assert_true(sexpressoEqual(sexpressoGetChildByPath(&Sexp, "myshit/a"), &Test));
	sexpressoDestroy(&Test);


	assert_false(sexpressoParse(&Test, "name you", NULL));
	assert_true(sexpressoEqual(sexpressoGetChildByPath(&Sexp, "myshit/b/name"), &Test));
	sexpressoDestroy(&Test);

	assert_false(sexpressoParse(&Test, "age 1", NULL));
	assert_true(sexpressoEqual(sexpressoGetChildByPath(&Sexp, "myshit/b/age"), &Test));
	sexpressoDestroy(&Test);

	assert_false(sexpressoParse(&Test, "b (name you) (age 1)", NULL));
	assert_true(sexpressoEqual(sexpressoGetChildByPath(&Sexp, "myshit/b"), &Test));
	sexpressoDestroy(&Test);

	assert_null(sexpressoGetChildByPath(&Sexp, "this/does/not/even/exist/dummy"));
	sexpressoDestroy(&Sexp);
}

int main(int argc, char** argv) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_empty_string),
		cmocka_unit_test(test_empty_sexp),
		cmocka_unit_test(test_multiple_empty_sexp),
		cmocka_unit_test(equality),
		cmocka_unit_test(inequality),
		cmocka_unit_test(string_literal),
		cmocka_unit_test(hierarchy_query),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
