/*
 * File:   t3.c
 * Author: Adamansky Anton <anton@adamansky.com>
 *
 * Created on Oct 26, 2012, 12:12:45 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "ejdb_private.h"
#include <locale.h>
#include <pthread.h>

#include "CUnit/Basic.h"

/*
 * CUnit Test Suite
 */

static EJDB *jb;
const int RS = 100000;
const int QRS = 100;
static bson* recs;

int init_suite(void) {
    assert(QRS < RS);
    jb = ejdbnew();
    if (!ejdbopen(jb, "dbt3", JBOWRITER | JBOCREAT | JBOTRUNC)) {
        return 1;
    }
    srandom(tcmstime());
    recs = malloc(RS * sizeof (bson));
    if (!recs) {
        return 1;
    }
    for (int i = 0; i < RS; ++i) {
        bson bs;
        bson_init(&bs);
        bson_append_long(&bs, "ts", tcmstime());
        char str[128];
        int len = 0;
        do {
            len = random() % 128;
        } while (len <= 0);
        str[0] = 'A' + (i % 26);
        for (int j = 1; j < len; ++j) {
            str[j] = 'a' + random() % 26;
        }
        str[len] = '\0';
        bson_append_string(&bs, "rstring", str);
        bson_finish(&bs);
        recs[i] = bs;
    }

    return 0;
}

int clean_suite(void) {
    ejdbclose(jb);
    ejdbdel(jb);
    for (int i = 0; i < RS; ++i) {
        bson_destroy(&recs[i]);
    }
    free(recs);
    return 0;
}

void testPerf1() {
    CU_ASSERT_PTR_NOT_NULL_FATAL(jb);
    CU_ASSERT_PTR_NOT_NULL_FATAL(recs);
    EJCOLL *coll = ejdbcreatecoll(jb, "pcoll1", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    unsigned long st = tcmstime();
    for (int i = 0; i < RS; ++i) {
        bson_oid_t oid;
        ejdbsavebson(coll, recs + i, &oid);
    }
    ejdbsyncoll(coll);
    fprintf(stderr, "\ntestPerf1(): SAVED %d BSON OBJECTS, TIME %lu ms\n", RS, tcmstime() - st);

    st = tcmstime();
    uint32_t acount = 0;
    int i;
    for (i = 0; i < QRS; ++i) {
        int idx = random() % QRS;
        bson *bs = recs + idx;
        assert(bs);
        EJQ *q = ejdbcreatequery(jb, bs, NULL, 0, NULL);
        assert(q);
        uint32_t count;
        ejdbqrysearch(coll, q, &count, JBQRYCOUNT, NULL);
        assert(count);
        if (count != 1) {
            fprintf(stderr, "CNT=%u\n", count);
        }
        acount += count;
        ejdbquerydel(q);
    }
    CU_ASSERT_TRUE(i <= acount);
    fprintf(stderr, "testPerf1(): %u QUERIES, TIME: %lu ms, PER QUERY TIME: %lu ms\n",
            i, tcmstime() - st, (unsigned long) ((tcmstime() - st) / QRS));

    st = tcmstime();
    CU_ASSERT_TRUE(ejdbsetindex(coll, "rstring", JBIDXSTR));
    fprintf(stderr, "testPerf1(): SET INDEX 'rstring' TIME: %lu ms\n", tcmstime() - st);

    st = tcmstime();
    acount = 0;
    for (i = 0; i < QRS; ++i) {
        int idx = random() % QRS;
        bson *bs = recs + idx;
        assert(bs);
        EJQ *q = ejdbcreatequery(jb, bs, NULL, 0, NULL);
        assert(q);
        uint32_t count;
        ejdbqrysearch(coll, q, &count, JBQRYCOUNT, NULL);
        assert(count);
        acount += count;
        ejdbquerydel(q);
    }
    CU_ASSERT_TRUE(i <= acount);
    fprintf(stderr, "testPerf1(): %u QUERIES WITH 'rstring' INDEX, TIME: %lu ms, PER QUERY TIME: %lu ms\n",
            i, tcmstime() - st, (unsigned long) ((tcmstime() - st) / QRS));

    ejdbrmcoll(jb, coll->cname, true);
}



//Race conditions

typedef struct {
    int id;
    EJDB *jb;

} TARGRACE;

static void eprint(EJDB *jb, int line, const char *func) {
    int ecode = ejdbecode(jb);
    fprintf(stderr, "%d: %s: error: %d: %s\n",
            line, func, ecode, ejdberrmsg(ecode));
}

static void *threadrace1(void *_tr) {
    const int iterations = 500;
    TARGRACE *tr = (TARGRACE*) _tr;
    bool err = false;

    bson bq;
    bson_init_as_query(&bq);
    bson_append_int(&bq, "tid", tr->id);
    bson_finish(&bq);

    bson_type bt;
    bson_iterator it;
    void *bsdata;
    bool saved = false;
    int lastcnt = 0;

    for (int i = 0; !err && i < iterations; ++i) {
        EJCOLL *coll = ejdbcreatecoll(jb, "threadrace1", NULL);
        CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
        EJQ *q = ejdbcreatequery(jb, &bq, NULL, 0, NULL);
        CU_ASSERT_PTR_NOT_NULL_FATAL(q);

        bson_oid_t oid2;
        bson_oid_t *oid = NULL;
        int cnt = 0;
        uint32_t count;
        TCLIST *res = NULL;

        if (ejdbecode(jb) != 0) {
            eprint(jb, __LINE__, "threadrace1");
            err = true;
            goto ffinish;
        }

        res = ejdbqrysearch(coll, q, &count, 0, NULL);

        if (ejdbecode(jb) != 0) {
            eprint(jb, __LINE__, "threadrace1.ejdbqrysearch");
            err = true;
            goto ffinish;
        }
        if (count != 1 && saved) {
            CU_ASSERT_TRUE_FATAL(false);
            goto ffinish;
        }
        if (count > 0) {
            bsdata = TCLISTVALPTR(res, 0);
            CU_ASSERT_PTR_NOT_NULL_FATAL(bsdata);
            bt = bson_find_from_buffer(&it, bsdata, "cnt");
            CU_ASSERT_EQUAL_FATAL(bt, BSON_INT);
            cnt = bson_iterator_int(&it);
            bt = bson_find_from_buffer(&it, bsdata, "_id");
            CU_ASSERT_EQUAL_FATAL(bt, BSON_OID);
            oid = bson_iterator_oid(&it);
            CU_ASSERT_PTR_NOT_NULL_FATAL(oid);
        }

        bson sbs;
        bson_init(&sbs);
        if (oid) {
            bson_append_oid(&sbs, "_id", oid);
        }
        bson_append_int(&sbs, "tid", tr->id);
        bson_append_int(&sbs, "cnt", ++cnt);
        bson_finish(&sbs);

        if (!ejdbsavebson(coll, &sbs, &oid2)) {
            eprint(jb, __LINE__, "threadrace1.ejdbsavebson");
            err = true;
        }
        bson_destroy(&sbs);
        lastcnt = cnt;
ffinish:
        if (res) tclistdel(res);
        if (q) ejdbquerydel(q);
    }
    bson_destroy(&bq);
    CU_ASSERT_EQUAL(lastcnt, iterations);
    //fprintf(stderr, "\nThread %d finished", tr->id);
    return err ? "error" : NULL;
}

void testRace1() {
    CU_ASSERT_PTR_NOT_NULL_FATAL(jb);
    const int tnum = 50;
    bool err = false;
    TARGRACE targs[tnum];
    pthread_t threads[tnum];

    EJCOLL *coll = ejdbcreatecoll(jb, "threadrace1", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(coll);
    if (!ejdbsetindex(coll, "tid", JBIDXNUM)) {
        eprint(jb, __LINE__, "testRace1");
        err = true;
    }
    if (err) {
        goto finish;
    }

    for (int i = 0; i < tnum; i++) {
        targs[i].jb = jb;
        targs[i].id = i;
        if (pthread_create(threads + i, NULL, threadrace1, targs + i) != 0) {
            eprint(jb, __LINE__, "pthread_create");
            targs[i].id = -1;
            err = true;
        }
    }

    for (int i = 0; i < tnum; i++) {
        if (targs[i].id == -1) continue;
        void *rv;
        if (pthread_join(threads[i], &rv) != 0) {
            eprint(jb, __LINE__, "pthread_join");
            err = true;
        } else if (rv) {
            err = true;
        }
    }

finish:
    CU_ASSERT_FALSE(err);
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("t3", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if (
            //(NULL == CU_add_test(pSuite, "testPerf1", testPerf1)) ||
            (NULL == CU_add_test(pSuite, "testRace1", testRace1))

            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    int ret = CU_get_error() || CU_get_number_of_failures();
    CU_cleanup_registry();
    return ret;
}
