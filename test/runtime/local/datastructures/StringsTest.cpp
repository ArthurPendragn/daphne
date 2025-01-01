#include <runtime/local/datastructures/DataObjectFactory.h>
#include <runtime/local/datastructures/DenseMatrix.h>
#include <runtime/local/datastructures/ValueTypeUtils.h>
#include <runtime/local/io/ReadCsv.h>
#include <runtime/local/kernels/EwBinarySca.h>

#include <tags.h>

#include <catch.hpp>

#include <cstdint>

#define TEST_NAME(opName) "Strings (" opName ")"

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("ReadCsv"), TAG_IO, (DenseMatrix), (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    size_t numRows = 5000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    REQUIRE(m->getNumRows() == numRows);
    REQUIRE(m->getNumCols() == numCols);

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("eq"), TAG_KERNELS, (DenseMatrix), (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m1 = nullptr;

    using DT = TestType;
    DT *m2 = nullptr;

    size_t numRows = 5000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);
    readCsv(m, filename, numRows, numCols, delim);

    DTRes *res = nullptr;
    ewBinaryMat<DTRes, DTArg, DTArg>((BinaryOpCode::EQ, res, m1, m2, nullptr);

    DataObjectFactory::destroy(m1);
    DataObjectFactory::destroy(m2);
}