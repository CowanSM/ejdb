/*
 * File:   newcunittest.c
 * Author: Adamansky Anton <anton@adamansky.com>
 *
 * Created on Oct 1, 2012, 3:13:44 PM
 */

#include "bson.h"


#include <stdio.h>
#include <stdlib.h>
#include "CUnit/Basic.h"
#include <assert.h>
#include "ejdb_private.h"
#include <locale.h>

/*
 * CUnit Test Suite
 */

static EJDB *jb;

int init_suite(void) {
    jb = ejdbnew();
    if (!ejdbopen(jb, "dbt2", JBOWRITER | JBOCREAT | JBOTRUNC)) {
        return 1;
    }
    return 0;
}

int clean_suite(void) {
    ejdbrmcoll(jb, "contacts", true);
    ejdbclose(jb);
    ejdbdel(jb);
    return 0;
}

void testAddData() {
    CU_ASSERT_PTR_NOT_NULL_FATAL(jb);
    bson_oid_t oid;
    EJCOLL *ccoll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL(ccoll);

    //Record 1
    bson a1;

    bson_init(&a1);
    bson_append_string(&a1, "name", "Антонов");
    bson_append_string(&a1, "phone", "333-222-333");
    bson_append_int(&a1, "age", 33);
    bson_append_long(&a1, "longscore", 0xFFFFFFFFFF01LL);
    bson_append_double(&a1, "dblscore", 0.333333);
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "city", "Novosibirsk");
    bson_append_string(&a1, "country", "Russian Federation");
    bson_append_string(&a1, "zip", "630090");
    bson_append_string(&a1, "street", "Pirogova");
    bson_append_int(&a1, "room", 334);
    bson_append_finish_object(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);
    bson_finish(&a1);
    ejdbsavebson(ccoll, &a1, &oid);
    bson_destroy(&a1);

    //Record 2
    bson_init(&a1);
    bson_append_string(&a1, "name", "Адаманский");
    bson_append_string(&a1, "phone", "444-123-333");
    bson_append_long(&a1, "longscore", 0xFFFFFFFFFF02LL);
    bson_append_double(&a1, "dblscore", 0.93);
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "city", "Novosibirsk");
    bson_append_string(&a1, "country", "Russian Federation");
    bson_append_string(&a1, "zip", "630090");
    bson_append_string(&a1, "street", "Pirogova");
    bson_append_finish_object(&a1);
    bson_append_start_array(&a1, "labels");
    bson_append_string(&a1, "0", "red");
    bson_append_string(&a1, "1", "green");
    bson_append_string(&a1, "2", "with gap, label");
    bson_append_finish_array(&a1);
    bson_append_start_array(&a1, "drinks");
    bson_append_int(&a1, "0", 4);
    bson_append_long(&a1, "1", 556667);
    bson_append_double(&a1, "2", 77676.22);
    bson_append_finish_array(&a1);

    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);
    ejdbsavebson(ccoll, &a1, &oid);
    bson_destroy(&a1);

    //Record 3
    bson_init(&a1);
    bson_append_string(&a1, "name", "Ivanov");
    bson_append_long(&a1, "longscore", 66);
    bson_append_double(&a1, "dblscore", 1.0);
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "city", "Petropavlovsk");
    bson_append_string(&a1, "country", "Russian Federation");
    bson_append_string(&a1, "zip", "683042");
    bson_append_string(&a1, "street", "Dalnaya");
    bson_append_finish_object(&a1);
    bson_append_start_array(&a1, "drinks");
    bson_append_int(&a1, "0", 41);
    bson_append_long(&a1, "1", 222334);
    bson_append_double(&a1, "2", 77676.22);
    bson_append_finish_array(&a1);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);

    CU_ASSERT_TRUE(ejdbsavebson(ccoll, &a1, &oid));
    bson_destroy(&a1);
}

void testSetIndex1() {
    EJCOLL *ccoll = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(ccoll);
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "ab.c.d", JBIDXSTR));
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "ab.c.d", JBIDXSTR | JBIDXNUM));
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "ab.c.d", JBIDXDROPALL));
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "address.zip", JBIDXSTR));
    CU_ASSERT_TRUE(ejdbsetindex(ccoll, "name", JBIDXSTR));

    //Insert new record with active index
    //Record 4
    bson a1;
    bson_oid_t oid;
    bson_init(&a1);
    bson_append_string(&a1, "name", "John Travolta");
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "country", "USA");
    bson_append_string(&a1, "zip", "4499995");
    bson_append_finish_object(&a1);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);
    CU_ASSERT_TRUE(ejdbsavebson(ccoll, &a1, &oid));
    bson_destroy(&a1);


    //Update record 4 with active index
    //Record 4
    bson_init(&a1);
    bson_append_oid(&a1, "_id", &oid);
    bson_append_string(&a1, "name", "John Travolta2");
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "country", "USA");
    bson_append_string(&a1, "zip", "4499996");
    bson_append_finish_object(&a1);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);

    CU_ASSERT_TRUE(ejdbsavebson(ccoll, &a1, &oid));
    CU_ASSERT_TRUE(ejdbrmbson(ccoll, &oid));
    bson_destroy(&a1);

    //Save Travolta again
    bson_init(&a1);
    bson_append_oid(&a1, "_id", &oid);
    bson_append_string(&a1, "name", "John Travolta");
    bson_append_start_object(&a1, "address");
    bson_append_string(&a1, "country", "USA");
    bson_append_string(&a1, "zip", "4499996");
    bson_append_string(&a1, "street", "Beverly Hills");
    bson_append_finish_object(&a1);
    bson_append_start_array(&a1, "labels");
    bson_append_string(&a1, "0", "yellow");
    bson_append_string(&a1, "1", "red");
    bson_append_string(&a1, "2", "black");
    bson_append_finish_array(&a1);
    bson_finish(&a1);
    CU_ASSERT_FALSE_FATAL(a1.err);
    CU_ASSERT_TRUE(ejdbsavebson(ccoll, &a1, &oid));
    bson_destroy(&a1);
}

void testQuery1() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "address.zip", "630090");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    //for (int i = 0; i < TCLISTNUM(q1res); ++i) {
    //    void *bsdata = TCLISTVALPTR(q1res, i);
    //    bson_print_raw(stderr, bsdata, 0);
    //}
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'saddress.zip'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);


    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);
    tcxstrdel(log);
}

void testQuery2() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "address.zip", "630090");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'saddress.zip'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }
    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);
    tcxstrdel(log);
}

void testQuery3() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.zip", JBIDXDROPALL));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "address.zip", "630090");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'sname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 20"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);
    tcxstrdel(log);
}

void testQuery4() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXDROPALL));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "address.zip", "630090");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }
    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);
    tcxstrdel(log);
}

void testQuery5() {

    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "labels", "red");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery6() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "labels", JBIDXARR));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "labels", "red");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'alabels'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 5"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"red\" 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("4499996", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery7() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "labels", "with gap, label");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'alabels'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 5"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"with gap, label\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery8() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //"labels" : {"$in" : ["yellow", "green"]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "labels");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "green");
    bson_append_string(&bsq1, "1", "yellow");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'alabels'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 5"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"green\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"yellow\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("yellow", TCLISTVALPTR(q1res, i), "labels.0"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.1"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery9() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "labels", JBIDXDROPALL));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_string(&bsq1, "labels", "red");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("4499996", TCLISTVALPTR(q1res, i), "address.zip"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("630090", TCLISTVALPTR(q1res, i), "address.zip"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }


    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery10() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.street", JBIDXSTR));

    //"address.street" : {"$in" : ["Pirogova", "Beverly Hills"]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.street");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "Pirogova");
    bson_append_string(&bsq1, "1", "Beverly Hills");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'saddress.street'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 6"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Beverly Hills", TCLISTVALPTR(q1res, i), "address.street"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Pirogova", TCLISTVALPTR(q1res, i), "address.street"));
        } else if (i == 2) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Pirogova", TCLISTVALPTR(q1res, i), "address.street"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }
    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery11() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.street", JBIDXDROPALL));

    //"address.street" : {"$in" : ["Pirogova", "Beverly Hills"]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.street");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "Pirogova");
    bson_append_string(&bsq1, "1", "Beverly Hills");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Beverly Hills", TCLISTVALPTR(q1res, i), "address.street"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Pirogova", TCLISTVALPTR(q1res, i), "address.street"));
        } else if (i == 2) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("Pirogova", TCLISTVALPTR(q1res, i), "address.street"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery12() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //"labels" : {"$in" : ["yellow", "green"]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "labels");
    bson_append_start_array(&bsq1, "$in");
    bson_append_string(&bsq1, "0", "green");
    bson_append_string(&bsq1, "1", "yellow");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("yellow", TCLISTVALPTR(q1res, i), "labels.0"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_string("green", TCLISTVALPTR(q1res, i), "labels.1"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery13() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //"drinks" : {"$in" : [4, 77676.22]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "drinks");
    bson_append_start_array(&bsq1, "$in");
    bson_append_int(&bsq1, "0", 4);
    bson_append_double(&bsq1, "1", 77676.22);
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));


    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_long(41, TCLISTVALPTR(q1res, i), "drinks.0"));
            CU_ASSERT_FALSE(bson_compare_long(222334, TCLISTVALPTR(q1res, i), "drinks.1"));
            CU_ASSERT_FALSE(bson_compare_double(77676.22, TCLISTVALPTR(q1res, i), "drinks.2"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_long(4, TCLISTVALPTR(q1res, i), "drinks.0"));
            CU_ASSERT_FALSE(bson_compare_long(556667, TCLISTVALPTR(q1res, i), "drinks.1"));
            CU_ASSERT_FALSE(bson_compare_double(77676.22, TCLISTVALPTR(q1res, i), "drinks.2"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery14() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "drinks", JBIDXARR));

    //"drinks" : {"$in" : [4, 77676.22]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "drinks");
    bson_append_start_array(&bsq1, "$in");
    bson_append_int(&bsq1, "0", 4);
    bson_append_double(&bsq1, "1", 77676.22);
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'adrinks'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 21"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"4\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"77676.220000\" 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery15() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXNUM));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_double(&bsq1, "dblscore", 0.333333);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 8"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery16() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXDROPALL));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_double(&bsq1, "dblscore", 0.333333);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery17() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXNUM));

    //"dblscore" : {"$bt" : [0.95, 0.3]}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_start_array(&bsq1, "$bt");
    bson_append_double(&bsq1, "0", 0.95);
    bson_append_double(&bsq1, "1", 0.333333);
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);


    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 13"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    //Second query
    tcxstrclear(log);
    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    ejdbquerydel(q1);

    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXDROPALL));

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_start_array(&bsq1, "$bt");
    bson_append_double(&bsq1, "0", 0.95);
    bson_append_double(&bsq1, "1", 0.333333);
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);


    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", 1); //ASC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery18() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXARR));

    //{"name" : {$strand : ["Travolta", "John"]}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$strand");
    bson_append_string(&bsq1, "0", "Travolta");
    bson_append_string(&bsq1, "1", "John");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    /*bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", 1); //ASC order on name
    bson_append_finish_object(&bshints);*/
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'aname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 4"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"John\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"Travolta\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    //Second query
    tcxstrclear(log);
    tclistdel(q1res);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXDROPALL));

    count = 0;
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery19() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXARR));

    //{"name" : {$stror : ["Travolta", "Антонов", "John"]}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$stror");
    bson_append_string(&bsq1, "0", "Travolta");
    bson_append_string(&bsq1, "1", "Антонов");
    bson_append_string(&bsq1, "2", "John");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'aname'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 5"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"John\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"Travolta\" 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "token occurrence: \"Антонов\" 1"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    //No-index query
    tcxstrclear(log);
    tclistdel(q1res);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "name", JBIDXDROPALL));

    count = 0;
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery20() {
    //dblscore
    //{'dblscore' : {'$gte' : 0.93}}
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXNUM));

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$gte", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", 1); //ASC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 10"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(1.0, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //GT

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$gt", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", 1); //ASC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 9"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //NOINDEX
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXDROPALL));

    //NOINDEX GTE
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$gte", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(1.0, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery21() {
    //{'dblscore' : {'lte' : 0.93}}
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXNUM));

    //LTE
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$lte", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 12"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //LT
    //{'dblscore' : {'$lt' : 0.93}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$lt", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'ndblscore'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 11"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    CU_ASSERT_TRUE(ejdbsetindex(contacts, "dblscore", JBIDXDROPALL));

    //NOINDEX GTE
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "dblscore");
    bson_append_double(&bsq1, "$lte", 0.93);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery22() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);
    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.country", JBIDXSTR));

    //{"address.country" : {$begin : "Ru"}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.country");
    bson_append_string(&bsq1, "$begin", "Ru");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'saddress.country'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX TCOP: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(1.0, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else if (i == 2) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    CU_ASSERT_TRUE(ejdbsetindex(contacts, "address.country", JBIDXDROPALL));

    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.country");
    bson_append_string(&bsq1, "$begin", "R");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "dblscore", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(1.0, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Адаманский", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.93, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else if (i == 2) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
            CU_ASSERT_FALSE(bson_compare_double(0.333333, TCLISTVALPTR(q1res, i), "dblscore"));
            CU_ASSERT_FALSE(bson_compare_string("Russian Federation", TCLISTVALPTR(q1res, i), "address.country"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery23() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_regex(&bsq1, "name", "(IvaNov$|АНтоноВ$)", "i");
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on dblscore
    bson_append_finish_object(&bshints);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == 0) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
        } else if (i == 1) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
        } else {
            CU_ASSERT_TRUE(false);
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery24() {
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson bshints;
    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_append_long(&bshints, "$skip", 1);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == TCLISTNUM(q1res) - 1) {
            CU_ASSERT_FALSE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", -1); //DESC order on name
    bson_append_finish_object(&bshints);
    bson_append_long(&bshints, "$skip", 1);
    bson_append_long(&bshints, "$max", 2);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: TRUE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: TRUE"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == TCLISTNUM(q1res) - 1) {
            CU_ASSERT_FALSE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //No order specified
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_long(&bshints, "$skip", 1);
    bson_append_long(&bshints, "$max", 2);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_long(&bshints, "$skip", 4);
    bson_append_long(&bshints, "$max", 2);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);
    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 4"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 0);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 0);

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson_init_as_query(&bshints);
    bson_append_start_object(&bshints, "$orderby");
    bson_append_int(&bshints, "name", 1); //ASC
    bson_append_finish_object(&bshints);
    bson_append_long(&bshints, "$skip", 3);
    bson_finish(&bshints);
    CU_ASSERT_FALSE_FATAL(bshints.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, &bshints);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        if (i == TCLISTNUM(q1res) - 1) {
            CU_ASSERT_FALSE(bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));
        }
    }

    bson_destroy(&bsq1);
    bson_destroy(&bshints);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

}

void testQuery25() { //$or
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    bson obs[2];
    bson_init_as_query(&obs[0]);
    bson_append_string(&obs[0], "name", "Ivanov");
    bson_finish(&obs[0]);
    CU_ASSERT_FALSE_FATAL(obs[0].err);

    bson_init_as_query(&obs[1]);
    bson_append_string(&obs[1], "name", "Антонов");
    bson_finish(&obs[1]);
    CU_ASSERT_FALSE_FATAL(obs[1].err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, obs, 2, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(
                !bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name") ||
                !bson_compare_string("Антонов", TCLISTVALPTR(q1res, i), "name"));

    }
    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    for (int i = 0; i < 2; ++i) {
        bson_destroy(&obs[i]);
    }
}

void testQuery26() { //$not $nin
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //{'address.city' : {$not : 'Novosibirsk'}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.city");
    bson_append_string(&bsq1, "$not", "Novosibirsk");
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(bson_compare_string("Novosibirsk", TCLISTVALPTR(q1res, i), "address.city"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //Double negation {'address.city' : {$not : {'$not' : 'Novosibirsk'}}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.city");
    bson_append_start_object(&bsq1, "$not");
    bson_append_string(&bsq1, "$not", "Novosibirsk");
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(!bson_compare_string("Novosibirsk", TCLISTVALPTR(q1res, i), "address.city"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //"name" : {"$nin" : ["John Travolta", "Ivanov"]}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "name");
    bson_append_start_array(&bsq1, "$nin");
    bson_append_string(&bsq1, "0", "John Travolta");
    bson_append_string(&bsq1, "1", "Ivanov");
    bson_append_finish_array(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 2"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 2);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 2);

    for (int i = 0; i < TCLISTNUM(q1res); ++i) {
        CU_ASSERT_TRUE(bson_compare_string("John Travolta", TCLISTVALPTR(q1res, i), "name"));
        CU_ASSERT_TRUE(bson_compare_string("Ivanov", TCLISTVALPTR(q1res, i), "name"));
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testQuery27() { //$exists
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    //{'address.room' : {$exists : true}}
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.room");
    bson_append_bool(&bsq1, "$exists", true);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 1);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 1);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);


    //{'address.room' : {$exists : true}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.room");
    bson_append_bool(&bsq1, "$exists", false);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);

    //{'address.room' : {$not :  {$exists : true}}} is equivalent to {'address.room' : {$exists : false}}
    bson_init_as_query(&bsq1);
    bson_append_start_object(&bsq1, "address.room");
    bson_append_start_object(&bsq1, "$not");
    bson_append_bool(&bsq1, "$exists", true);
    bson_append_finish_object(&bsq1);
    bson_append_finish_object(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    count = 0;
    log = tcxstrnew();
    q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAX: 4294967295"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "SKIP: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "COUNT ONLY: NO"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "MAIN IDX: 'NONE'"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ORDER FIELDS: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "ACTIVE CONDITIONS: 1"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "$OR QUERIES: 0"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FETCH ALL: FALSE"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RUN FULLSCAN"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS SIZE: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "RS COUNT: 3"));
    CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log), "FINAL SORTING: FALSE"));
    CU_ASSERT_EQUAL(count, 3);
    CU_ASSERT_TRUE(TCLISTNUM(q1res) == 3);

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

void testOIDSMatching() { //OID matching
    EJCOLL *contacts = ejdbcreatecoll(jb, "contacts", NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(contacts);

    bson_type bt;
    bson bsq1;
    bson_init_as_query(&bsq1);
    bson_finish(&bsq1);
    CU_ASSERT_FALSE_FATAL(bsq1.err);

    EJQ *q1 = ejdbcreatequery(jb, &bsq1, NULL, 0, NULL);
    CU_ASSERT_PTR_NOT_NULL_FATAL(q1);

    uint32_t count = 0;
    TCXSTR *log = tcxstrnew();
    TCLIST *q1res = ejdbqrysearch(contacts, q1, &count, 0, log);
    CU_ASSERT_TRUE(count > 0);
    //fprintf(stderr, "%s", TCXSTRPTR(log));

    for (int i = 0; i < TCLISTNUM(q1res); ++i) { //first
        char soid[25];
        bson_oid_t *oid;
        void *bsdata = TCLISTVALPTR(q1res, i);
        bson_iterator it2;
        bt = bson_find_from_buffer(&it2, bsdata, JDBIDKEYNAME);
        CU_ASSERT_EQUAL_FATAL(bt, BSON_OID);
        oid = bson_iterator_oid(&it2);
        bson_oid_to_string(oid, soid);
        //fprintf(stderr, "\nOID: %s", soid);

        //OID in string form maching
        bson bsq2;
        bson_init_as_query(&bsq2);

        if (i % 2 == 0) {
            bson_append_string(&bsq2, JDBIDKEYNAME, soid);
        } else {
            bson_append_oid(&bsq2, JDBIDKEYNAME, oid);
        }

        bson_finish(&bsq2);
        CU_ASSERT_FALSE_FATAL(bsq2.err);

        TCXSTR *log2 = tcxstrnew();
        EJQ *q2 = ejdbcreatequery(jb, &bsq2, NULL, 0, NULL);
        TCLIST *q2res = ejdbqrysearch(contacts, q2, &count, 0, log2);
        CU_ASSERT_PTR_NOT_NULL(strstr(TCXSTRPTR(log2), "PRIMARY KEY MATCHING:"));
        CU_ASSERT_EQUAL(count, 1);

        tcxstrdel(log2);
        ejdbquerydel(q2);
        tclistdel(q2res);
        bson_destroy(&bsq2);
    }

    bson_destroy(&bsq1);
    tclistdel(q1res);
    tcxstrdel(log);
    ejdbquerydel(q1);
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("t2", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "testAddData", testAddData)) ||
            (NULL == CU_add_test(pSuite, "testSetIndex1", testSetIndex1)) ||
            (NULL == CU_add_test(pSuite, "testQuery1", testQuery1)) ||
            (NULL == CU_add_test(pSuite, "testQuery2", testQuery2)) ||
            (NULL == CU_add_test(pSuite, "testQuery3", testQuery3)) ||
            (NULL == CU_add_test(pSuite, "testQuery4", testQuery4)) ||
            (NULL == CU_add_test(pSuite, "testQuery5", testQuery5)) ||
            (NULL == CU_add_test(pSuite, "testQuery6", testQuery6)) ||
            (NULL == CU_add_test(pSuite, "testQuery7", testQuery7)) ||
            (NULL == CU_add_test(pSuite, "testQuery8", testQuery8)) ||
            (NULL == CU_add_test(pSuite, "testQuery9", testQuery9)) ||
            (NULL == CU_add_test(pSuite, "testQuery10", testQuery10)) ||
            (NULL == CU_add_test(pSuite, "testQuery11", testQuery11)) ||
            (NULL == CU_add_test(pSuite, "testQuery12", testQuery12)) ||
            (NULL == CU_add_test(pSuite, "testQuery13", testQuery13)) ||
            (NULL == CU_add_test(pSuite, "testQuery14", testQuery14)) ||
            (NULL == CU_add_test(pSuite, "testQuery15", testQuery15)) ||
            (NULL == CU_add_test(pSuite, "testQuery16", testQuery16)) ||
            (NULL == CU_add_test(pSuite, "testQuery17", testQuery17)) ||
            (NULL == CU_add_test(pSuite, "testQuery18", testQuery18)) ||
            (NULL == CU_add_test(pSuite, "testQuery19", testQuery19)) ||
            (NULL == CU_add_test(pSuite, "testQuery20", testQuery20)) ||
            (NULL == CU_add_test(pSuite, "testQuery21", testQuery21)) ||
            (NULL == CU_add_test(pSuite, "testQuery22", testQuery22)) ||
            (NULL == CU_add_test(pSuite, "testQuery23", testQuery23)) ||
            (NULL == CU_add_test(pSuite, "testQuery24", testQuery24)) ||
            (NULL == CU_add_test(pSuite, "testQuery25", testQuery25)) ||
            (NULL == CU_add_test(pSuite, "testQuery26", testQuery26)) ||
            (NULL == CU_add_test(pSuite, "testQuery27", testQuery27)) ||
            (NULL == CU_add_test(pSuite, "testOIDSMatching", testOIDSMatching))
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
