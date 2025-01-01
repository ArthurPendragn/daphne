#include <runtime/local/datastructures/DataObjectFactory.h>
#include <runtime/local/datastructures/DenseMatrix.h>
#include <runtime/local/datastructures/ValueTypeUtils.h>
#include <runtime/local/io/ReadCsv.h>
#include <runtime/local/kernels/EwBinaryMat.h>

#include <tags.h>

#include <catch.hpp>

#include <cstdint>

#define TEST_NAME(opName) "Strings (" opName ")"

template <class DTArg, class DTRes>
void StringTestEwBinaryMat(BinaryOpCode opCode, const DTArg *lhs, const DTArg *rhs) {
    DTRes *res = nullptr;
    ewBinaryMat<DTRes, DTArg, DTArg>(opCode, res, lhs, rhs, nullptr);
    DataObjectFactory::destroy(res);
}

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
    using DTRes = DenseMatrix<int64_t>;

    DT *m1 = nullptr;
    DT *m2 = nullptr;

    size_t numRows = 5000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';

    readCsv(m1, filename, numRows, numCols, delim);
    readCsv(m2, filename, numRows, numCols, delim);

    DTRes *res = nullptr;
    StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::EQ, m1, m2, nullptr);

    DataObjectFactory::destroy(m1);
    DataObjectFactory::destroy(m2);
}