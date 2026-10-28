/* Build 26.10.27.1 08:12PM */
/*****************************************************************************
 *   TEST_UTIL.CPP                 Dice for Teleconference - unit tests      *
 *                                                                           *
 *   Copyright (C) 2026 Elwynor Technologies.                                *
 *                                                                           *
 *   Tests the helpers in DC_UTIL.CPP, the parts that can be checked away    *
 *   from a BBS: reading a dice spec, the random numbers, laying out the     *
 *   results, the channel list, and reading numbers and yes/no answers       *
 *   typed in the settings editor.                                           *
 *                                                                           *
 *   Build and run: TESTS\RUN_TESTS.CMD.  Exit code 0 = all passed.          *
 *                                                                           *
 *   Licensed under the GNU Affero General Public License v3.0.              *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "DC_UTIL.H"

static int fails = 0;
static int checks = 0;

static void
check(int ok, const char *what)
{
    checks++;
    if (!ok) {
        fails++;
        printf("FAIL: %s\n", what);
    }
}

static void
checkstr(const char *got, const char *want, const char *what)
{
    checks++;
    if (strcmp(got, want) != 0) {
        fails++;
        printf("FAIL: %s\n      got  [%s]\n      want [%s]\n", what, got, want);
    }
}

/* Parse with the module's usual limits (50 dice, 100 sides, d6 default). */
static int
parse(const char *s, int *n, int *sides)
{
    *n = *sides = -1;
    return(dc_parsedice(s, 50, 100, 6, n, sides));
}

static void
test_parsedice(void)
{
    int n, s;

    check(parse("3", &n, &s) == DC_OK && n == 3 && s == 6, "a bare count rolls d6");
    check(parse("D20", &n, &s) == DC_OK && n == 1 && s == 20, "D20 is one die");
    check(parse("d20", &n, &s) == DC_OK && n == 1 && s == 20, "case does not matter");
    check(parse("2D8", &n, &s) == DC_OK && n == 2 && s == 8, "2D8");
    check(parse(" 2 d 8 ", &n, &s) == DC_OK && n == 2 && s == 8, "spaces are ignored");
    check(parse("50d100", &n, &s) == DC_OK && n == 50 && s == 100, "limits are allowed");
    check(parse("1d2", &n, &s) == DC_OK && n == 1 && s == 2, "a coin");

    check(parse("0", &n, &s) == DC_BADN, "zero dice");
    check(parse("51", &n, &s) == DC_BADN, "one over the dice limit");
    check(parse("99999", &n, &s) == DC_BADN, "absurd count cannot overflow");
    check(parse("2D1", &n, &s) == DC_BADSIDES, "a one-sided die");
    check(parse("2D0", &n, &s) == DC_BADSIDES, "a no-sided die");
    check(parse("2D101", &n, &s) == DC_BADSIDES, "one over the sides limit");
    check(parse("2D99999", &n, &s) == DC_BADSIDES, "absurd sides cannot overflow");

    check(parse("", &n, &s) == DC_BADFMT, "nothing to roll");
    check(parse("D", &n, &s) == DC_BADFMT, "D with no number");
    check(parse("abc", &n, &s) == DC_BADFMT, "words");
    check(parse("3D8x", &n, &s) == DC_BADFMT, "trailing characters are refused");
    check(parse("3 4", &n, &s) == DC_BADFMT, "two numbers");
    check(parse("-3", &n, &s) == DC_BADFMT, "a negative count");

    /* A failed parse must not touch the outputs. */
    n = 7; s = 7;
    check(dc_parsedice("0", 50, 100, 6, &n, &s) == DC_BADN && n == 7 && s == 7,
          "a failed parse leaves n and sides alone");

    /* The module limits come from the settings, not constants. */
    check(dc_parsedice("10", 5, 100, 6, &n, &s) == DC_BADN, "a lower dice limit applies");
    check(dc_parsedice("D50", 50, 20, 6, &n, &s) == DC_BADSIDES, "a lower sides limit applies");
}

static void
test_rand(void)
{
    int seen[7] = { 0 }, i, r, ok = 1, big = 1;

    for (i = 0; i < 6000; i++) {
        r = dc_rand(6);
        if (r < 0 || r > 5) {
            ok = 0;
        }
        else {
            seen[r]++;
        }
    }
    check(ok, "dc_rand(6) stays in 0..5");
    ok = 1;
    for (i = 0; i < 6; i++) {
        /* Expect about 1000 each; a broken generator is nowhere near. */
        if (seen[i] < 800 || seen[i] > 1200) {
            ok = 0;
        }
    }
    check(ok, "dc_rand(6) is roughly even");
    for (i = 0; i < 1000; i++) {
        r = dc_rand(1000);
        if (r < 0 || r > 999) {
            big = 0;
        }
    }
    check(big, "dc_rand(1000) stays in range");
    check(dc_rand(1) == 0, "dc_rand(1) is always 0");
    check(dc_rand(0) == 0 && dc_rand(-4) == 0, "dc_rand with no range gives 0");
}

static void
test_fmtrolls(void)
{
    char b[64];
    int one[] = { 4 }, two[] = { 4, 2 }, three[] = { 4, 2, 6 };
    int many[100], i;

    check(dc_fmtrolls(b, sizeof(b), one, 1) == 1, "one roll fits");
    checkstr(b, "4", "one roll");
    dc_fmtrolls(b, sizeof(b), two, 2);
    checkstr(b, "4 and 2", "two rolls");
    dc_fmtrolls(b, sizeof(b), three, 3);
    checkstr(b, "4, 2 and 6", "three rolls");

    for (i = 0; i < 100; i++) {
        many[i] = 100;
    }
    check(dc_fmtrolls(b, sizeof(b), many, 100) == 0, "too many rolls report no fit");
    checkstr(b, "", "and leave the list empty");
    check(dc_fmtrolls(b, 0, one, 1) == 0, "a zero-size buffer is refused");

    /* The edge: exactly filling the buffer must not overrun it. */
    check(dc_fmtrolls(b, 6, three, 3) == 0, "\"4, 2 and 6\" does not fit in 6");
    check(dc_fmtrolls(b, 10, three, 3) == 0, "...nor in 10: it is 10 characters plus a terminator");
    check(dc_fmtrolls(b, 11, three, 3) == 1, "...but fits in 11");
    checkstr(b, "4, 2 and 6", "...unharmed at the edge");
}

static void
test_cleantext(void)
{
    char b[64];

    dc_cleantext(b, sizeof(b), "  OFF   now  ", 150);
    checkstr(b, "OFF now", "spaces collapse and trim");
    dc_cleantext(b, sizeof(b), "red \x1b[31mtext\x07!", 150);
    checkstr(b, "red [31mtext!", "control characters are dropped");
    dc_cleantext(b, sizeof(b), "abcdefghij", 4);
    checkstr(b, "abcd", "maxlen applies");
    dc_cleantext(b, 6, "abcdefghij", 150);
    checkstr(b, "abcde", "size applies");
    dc_cleantext(b, sizeof(b), "", 150);
    checkstr(b, "", "empty stays empty");
}

static void
test_chanok(void)
{
    check(dc_chanok("", "Main"), "empty list allows every channel");
    check(dc_chanok("   ", "Main"), "blank list allows every channel");
    check(dc_chanok("*", "MajorLink"), "* allows every channel");
    check(dc_chanok("MajorLink", "majorlink"), "match ignores case");
    check(dc_chanok("Main, MajorLink ,Games", "MajorLink"), "match in the middle, spaces trimmed");
    check(dc_chanok("Main,MajorLink", "Main"), "match first");
    check(!dc_chanok("Main,MajorLink", "Major"), "no partial-name match");
    check(!dc_chanok("Main,MajorLink", "MajorLinkX"), "no longer-name match");
    check(!dc_chanok("Main", ""), "an empty channel name never matches a list");
    check(!dc_chanok("Main,,", "Games"), "empty entries match nothing");
    check(dc_chanok("Main, *", "Games"), "a * entry in a list allows all");
}

static void
test_parse(void)
{
    long v = 0;

    check(dc_parsenum(" 30 ", &v) && v == 30, "parsenum reads 30");
    check(dc_parsenum("-1", &v) && v == -1, "parsenum reads a sign");
    check(!dc_parsenum("30s", &v), "parsenum rejects trailing letters");
    check(!dc_parsenum("", &v), "parsenum rejects nothing");
    check(!dc_parsenum("abc", &v), "parsenum rejects words");

    check(dc_parsebool("yes") == 1 && dc_parsebool("ON") == 1 && dc_parsebool("1") == 1,
          "parsebool yes");
    check(dc_parsebool("No") == 0 && dc_parsebool("off") == 0 && dc_parsebool("0") == 0,
          "parsebool no");
    check(dc_parsebool("maybe") == -1, "parsebool rejects other words");
}

int
main(void)
{
    test_parsedice();
    test_rand();
    test_fmtrolls();
    test_cleantext();
    test_chanok();
    test_parse();
    printf("%d checks, %d failed\n", checks, fails);
    return(fails == 0 ? 0 : 1);
}
