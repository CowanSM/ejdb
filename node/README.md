EJDB NodeJS
================================================

Installation
--------------------------------
**System libraries:**

* g++
* zlib

On Debian/Ubuntu linux you can install it as follows:

```sh
sudo apt-get install g++ zlib1g zlib1g-dev
```

**Installation from node package manager on linux/macos:**

    npm install ejdb

**[Installing EJDB NodeJS module on windows](https://github.com/Softmotions/ejdb/blob/master/tcejdb/WIN32.md#ejdb-nodejs-module-installation)**


EJDB NodeJS samples
---------------------------------

* [node/samples](https://github.com/Softmotions/ejdb/tree/master/node/samples)
* [node/tests](https://github.com/Softmotions/ejdb/tree/master/node/tests)
* [nwk-ejdb-address-book](https://github.com/Softmotions/nwk-ejdb-address-book)


EJDB NodeJS API
----------------------------------

<a name="open" />
### EJDB.open(dbFile, openMode)

Open database. Return database instance handle object.
<br/>Default open mode: `JBOWRITER | JBOCREAT`.
<br/>This is blocking function.

__Arguments__

 * {String} dbFile Database main file name
 * {Number} `[openMode=JBOWRITER | JBOCREAT]` Bitmast of open modes:
       - `JBOREADER` Open as a reader.
       - `JBOWRITER` Open as a writer.
       - `JBOCREAT` Create if db file not exists
       - `JBOTRUNC` Truncate db.

---------------------------------------

<a name="close" />
### close()

Close database.
<br/>If database was not opened it does nothing.
<br/>This is blocking function.

---------------------------------------

<a name="isOpen"/>
### isOpen()
Check if database in opened state.

---------------------------------------

<a name="ensureCollection"/>
### ensureCollection(cname, copts)

Automatically creates new collection if it does't exists.
Collection options `copts` applied only for newly created collection.
For existing collections `copts` takes no effect.

Collection options (copts):

 * cachedrecords : Max number of cached records in shared memory segment. Default: 0
 * records : Estimated number of records in this collection. Default: 65535.
 * large : Specifies that the size of the database can be larger than 2GB. Default: false
 * compressed : If true collection records will be compressed with DEFLATE compression. Default: false.

<br/>This is blocking function.

__Arguments__

 * {String} cname Name of collection.
 * {Object} `[copts]` Collection options.

---------------------------------------


<a name="dropCollection"/>
### dropCollection(cname, prune, cb)

Drop collection.

Call variations:

    dropCollection(cname)
    dropCollection(cname, cb)
    dropCollection(cname, prune, cb)

__Arguments__

 * {String} cname Name of collection.
 * {Boolean} `[prune=false]` If true the collection data will erased from disk.
 * {Function} `[cb]` Callback args: (error)

---------------------------------------

<a name="save"/>
### save(cname, jsarr, cb)

Save/update specified JSON objects in the collection.
If collection with `cname` does not exists it will be created.

Each persistent object has unique identifier (OID) placed in the `_id` property.
If a saved object does not have  `_id` it will be autogenerated.
To identify and update object it should contains `_id` property.

If callback is not provided this function will be synchronous.

Call variations:

    save(cname, <json object>|<Array of json objects>, [options] [cb])
    save(cname, <json object>|<Array of json objects>, [cb])

NOTE: Field names of passed JSON objects may not contain `$` and `.` characters,
      error condition will be fired in this case.

__Arguments__

 * {String} **cname** Name of collection.
 * {Array|Object} jsarr Signle JSON object or array of JSON objects to save
 * {Function} `[cb]` Callback args: (error, {Array} of OIDs for saved objects)

__Return__

 * {Array} of OIDs of saved objects in synchronous mode otherwise returns {undefined}.

--------------------------------------

<a name="load"/>
### load(cname, oid, cb)

Loads JSON object identified by OID from the collection.
If callback is not provided this function will be synchronous.

__Arguments__

 * {String} cname Name of collection
 * {String} oid Object identifier (OID)
 * {Function} cb  Callback args: (error, obj)
        `obj`:  Retrieved JSON object or NULL if it is not found.

__Return__

 * JSON object or {null} if it is not found in synchronous mode otherwise return {undefined}.

--------------------------------------

<a name="remove"/>
### remove(cname, oid, cb)

Removes JSON object from the collection.
If callback is not provided this function will be synchronous.

__Arguments__

 * {String} cname Name of collection
 * {String} oid Object identifier (OID)
 * {Function} cb  Callback args: (error)


--------------------------------------

<a name="find"/>
### find(cname, qobj, orarr, hints, cb)
Execute query on collection.
EJDB queries inspired by MongoDB (mongodb.org) and follows same philosophy.

    Supported queries:
      - Simple matching of String OR Number OR Array value:
          -   {'fpath' : 'val', ...}
      - $not Negate operation.
          -   {'fpath' : {'$not' : val}} //Field not equal to val
          -   {'fpath' : {'$not' : {'$begin' : prefix}}} //Field not begins with val
      - $begin String starts with prefix
          -   {'fpath' : {'$begin' : prefix}}
      - $gt, $gte (>, >=) and $lt, $lte for number types:
          -   {'fpath' : {'$gt' : number}, ...}
      - $bt Between for number types:
          -   {'fpath' : {'$bt' : [num1, num2]}}
      - $in String OR Number OR Array val matches to value in specified array:
          -   {'fpath' : {'$in' : [val1, val2, val3]}}
      - $nin - Not IN
      - $strand String tokens OR String array val matches all tokens in specified array:
          -   {'fpath' : {'$strand' : [val1, val2, val3]}}
      - $stror String tokens OR String array val matches any token in specified array:
          -   {'fpath' : {'$stror' : [val1, val2, val3]}}
      - $exists Field existence matching:
          -   {'fpath' : {'$exists' : true|false}}
      - $icase Case insensitive string matching:
          -  {'fpath' : {'$icase' : 'val1'}} //icase matching
          Ignore case matching with '$in' operation:
          -  {'name' : {'$icase' : {'$in' : ['tHéâtre - театр', 'heLLo WorlD']}}}
          For case insensitive matching you can create special type of string index.
      - $elemMatch The $elemMatch operator matches more than one component within an array element.
          -  { array: { $elemMatch: { value1 : 1, value2 : { $gt: 1 } } } }
          Restriction: only one $elemMatch allowed in context of one array field.

      - Queries can be used to update records:

        $set Field set operation.
            - {.., '$set' : {'field1' : val1, 'fieldN' : valN}}
        $upsert Atomic upsert. If matching records are found it will be '$set' operation,
                otherwise new record will be inserted with fields specified by argment object.
           - {.., '$upsert' : {'field1' : val1, 'fieldN' : valN}}
        $inc Increment operation. Only number types are supported.
            - {.., '$inc' : {'field1' : number, ...,  'field1' : number}
        $dropall In-place record removal operation.
            - {.., '$dropall' : true}
        $addToSet Atomically adds value to the array only if its not in the array already.
                    If containing array is missing it will be created.
            - {.., '$addToSet' : {'fpath' : val1, 'fpathN' : valN, ...}}
        $addToSetAll Batch version if $addToSet
            - {.., '$addToSetAll' : {'fpath' : [array of values to add], ...}}
        $pull Atomically removes all occurrences of value from field, if field is an array.
            - {.., '$pull' : {'fpath' : val1, 'fpathN' : valN, ...}}
        $pullAll Batch version of $pull
            - {.., '$pullAll' : {'fpath' : [array of values to remove], ...}}

    NOTE: It is better to execute update queries with `$onlycount=true` hint flag
         or use the special `update()` method to avoid unnecessarily rows fetching.

    NOTE: Negate operations: $not and $nin not using indexes
          so they can be slow in comparison to other matching operations.

    NOTE: Only one index can be used in search query operation.

    NOTE: If callback is not provided this function will be synchronous.

    QUERY HINTS (specified by `hints` argument):
      - $max Maximum number in the result set
      - $skip Number of skipped results in the result set
      - $orderby Sorting order of query fields.
      - $onlycount true|false If `true` only count of matching records will be returned
                              without placing records in result set.
      - $fields Set subset of fetched fields
           If a field presented in $orderby clause it will be forced to include in resulting records.
           Example:
           hints:    {
                       "$orderby" : { //ORDER BY field1 ASC, field2 DESC
                           "field1" : 1,
                           "field2" : -1
                       },
                       "$fields" : { //SELECT ONLY {_id, field1, field2}
                           "field1" : 1,
                           "field2" : 1
                       }
                     }

    Many C API query examples can be found in `tcejdb/testejdb/t2.c` test case.

    To traverse selected records cursor object is used:
      - Cursor#next() Move cursor to the next record and returns true if next record exists.
      - Cursor#hasNext() Returns true if cursor can be placed to the next record.
      - Cursor#field(name) Retrieve value of the specified field of the current JSON object record.
      - Cursor#object() Retrieve whole JSON object with all fields.
      - Cursor#reset() Reset cursor to its initial state.
      - Cursor#length Read-only property: Number of records placed into cursor.
      - Cursor#pos Read/Write property: You can set cursor position: 0 <= pos < length
      - Cursor#close() Closes cursor and free cursor resources. Cursor cant be used in closed state.

    Call variations of find():
       - find(cname, [cb])
       - find(cname, qobj, [cb])
       - find(cname, qobj, hints, [cb])
       - find(cname, qobj, qobjarr, [cb])
       - find(cname, qobj, qobjarr, hints, [cb])

 __Arguments__

 * {String} cname Name of collection
 * {Object} qobj Main JSON query object
 * {Array} `[orarr]` Array of additional OR query objects (joined with OR predicate).
 * {Object} `[hints]` JSON object with query hints.
 * {Function} cb Callback args: (error, cursor, count)
            `cursor`: Cursor object to traverse records
      qobj      `count`:  Total number of selected records

__Return__

 * If callback is provided returns {undefined}
 * If no callback and `$onlycount` hint is set returns count {Number}.
 * If no callback and no `$onlycount` hint returns cursor {Object}.

 --------------------------------------------

<a name="findOne"/>
### findOne(cname, qobj, orarr, hints, cb)
Same as #find() but retrieves only one matching JSON object.

Call variations of findOne():

    findOne(cname, [cb])
    findOne(cname, qobj, [cb])
    findOne(cname, qobj, hints, [cb])
    findOne(cname, qobj, qobjarr, [cb])
    findOne(cname, qobj, qobjarr, hints, [cb])

__Arguments__

 * {String} cname Name of collection
 * {Object} qobj Main JSON query object
 * {Array} `[orarr]` Array of additional OR query objects (joined with OR predicate).
 * {Object} `[hints]` JSON object with query hints.
 * {Function} cb Callback args: (error, obj)
             `obj`  Retrieved JSON object or NULL if it is not found.

__Return__

 * If callback is provided returns {undefined}
 * If no callback is provided returns found {Object} or {null}

-----------------------------------
<a name="findOne"/>
### update(cname, qobj, orarr, hints, cb)
Convenient method to execute update queries.

 * `$set` Field set operation:
    - {some fields for selection, '$set' : {'field1' : {obj}, ...,  'field1' : {obj}}}
 * `$upsert` Atomic upsert. If matching records are found it will be '$set' operation,
 otherwise new record will be inserted with fields specified by argment object.
    - {.., '$upsert' : {'field1' : val1, 'fieldN' : valN}}
 * `$inc` Increment operation. Only number types are supported.
    - {some fields for selection, '$inc' : {'field1' : number, ...,  'field1' : {number}}
 * `$dropall` In-place record removal operation.
    - {some fields for selection, '$dropall' : true}
 * `$addToSet` | `$addToSetAll` Atomically adds value to the array only if its not in the array already.
 If containing array is missing it will be created.
    - {.., '$addToSet' : {'fpath' : val1, 'fpathN' : valN, ...}}
 * `$pull` | `pullAll` Atomically removes all occurrences of value from field, if field is an array.
    - {.., '$pull' : {'fpath' : val1, 'fpathN' : valN, ...}}

Call variations of update():

    update(cname, [cb])
    update(cname, qobj, [cb])
    update(cname, qobj, hints, [cb])
    update(cname, qobj, qobjarr, [cb])
    update(cname, qobj, qobjarr, hints, [cb])

__Arguments__

 * {String} cname Name of collection
 * {Object} qobj Update JSON query object
 * {Array} `[orarr]` Array of additional OR query objects (joined with OR predicate).
 * {Object} `[hints]` JSON object with query hints.
 * {Function} cb Callback args: (error, count)
             `count`  The number of updated records.

__Return__

 * If callback is provided returns {undefined}.
 * If no callback is provided returns {Number} of updated objects.


-----------------------------------

<a name="count"/>
### count(cname, qobj, orarr, hints, cb)
Convenient count(*) operation.

Call variations of count():

    count(cname, [cb])
    count(cname, qobj, [cb])
    count(cname, qobj, hints, [cb])
    count(cname, qobj, qobjarr, [cb])
    count(cname, qobj, qobjarr, hints, [cb])

__Arguments__

 * {String} cname Name of collection
 * {Object} qobj Main JSON query object
 * {Array} `[orarr]` Array of additional OR query objects (joined with OR predicate).
 * {Object} `[hints]` JSON object with query hints.
 * {Function} cb Callback args: (error, count)
              `count`:  Number of matching records.

__Return__

 * If callback is provided returns {undefined}.
 * If no callback is provided returns {Number} of matched object.

-----------------------------------

<a name="sync"/>
### sync(cb)
Synchronize entire EJDB database with disk.

__Arguments__

 * {Function} cb Callback args: (error)

-----------------------------------

<a name="dropIndexes"/>
### dropIndexes(cname, path, cb)
Drop indexes of all types for JSON field path.

__Arguments__

 * {String} cname Name of collection
 * {String} path  JSON field path
 * {Function} `[cb]` Optional callback function. Callback args: (error)

------------------------------------

<a name="optimizeIndexes"/>
### optimizeIndexes(cname, path, cb)
Optimize indexes of all types for JSON field path.
Performs B+ tree index file optimization.

 __Arguments__

  * {String} cname Name of collection
  * {String} path  JSON field path
  * {Function} `[cb]` Optional callback function. Callback args: (error)

-----------------------------------

<a name="ensureIndex"/>
### ensureStringIndex(cname, path, cb)
### ensureIStringIndex(cname, path, cb)
### ensureNumberIndex(cname, path, cb)
### ensureArrayIndex(cname, path, cb)

Ensure index presence of String|Number|Array type for JSON field path.
`IString` is the special type of String index for case insensitive matching.

 __Arguments__

  * {String} cname Name of collection
  * {String} path  JSON field path
  * {Function} `[cb]` Optional callback function. Callback args: (error)

-----------------------------------

<a name="rebuildIndex"/>
### rebuildStringIndex(cname, path, cb)
### rebuildIStringIndex(cname, path, cb)
### rebuildNumberIndex(cname, path, cb)
### rebuildArrayIndex(cname, path, cb)

Rebuild index of String|Number|Array type for JSON field path.
`IString` is the special type of String index for case insensitive matching.

 __Arguments__

  * {String} cname Name of collection
  * {String} path  JSON field path
  * {Function} `[cb]` Optional callback function. Callback args: (error)

-----------------------------------

<a name="dropIndex"/>
### dropStringIndex(cname, path, cb)
### dropIStringIndex(cname, path, cb)
### dropNumberIndex(cname, path, cb)
### dropArrayIndex(cname, path, cb)

Drop index of String|Number|Array type for JSON field path.
`IString` is the special type of String index for case insensitive matching.

 __Arguments__

  * {String} cname Name of collection
  * {String} path  JSON field path
  * {Function} `[cb]` Optional callback function. Callback args: (error)

-----------------------------------
