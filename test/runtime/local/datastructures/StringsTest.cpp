#include <runtime/local/datastructures/DataObjectFactory.h>
#include <runtime/local/datastructures/DenseMatrix.h>
#include <runtime/local/datastructures/Umbra.h>
#include <runtime/local/datastructures/ValueTypeUtils.h>
#include <runtime/local/io/ReadCsv.h>
#include <runtime/local/kernels/EwBinaryMat.h>
#include <runtime/local/kernels/EwBinarySca.h>

#include <tags.h>

#include <catch.hpp>

#include <cstdint>

#define TEST_NAME(opName) "Strings (" opName ")"
#define PARTIAL_STRING_VALUE_TYPES std::string, Umbra_t

template <class DTArg, class DTRes>
void StringTestEwBinaryMat(BinaryOpCode opCode, const DTArg *lhs, const DTArg *rhs) {
    DTRes *res = nullptr;
    ewBinaryMat<DTRes, DTArg, DTArg>(opCode, res, lhs, rhs, nullptr);
    DataObjectFactory::destroy(res);
}

template <BinaryOpCode opCode> void StringTestEwBinarySca(std::string lhs, std::string rhs, int64_t exp) {
    EwBinarySca<opCode, int64_t, std::string, std::string>::apply(lhs, rhs, nullptr);
    ewBinarySca<int64_t, std::string, std::string>(opCode, lhs, rhs, nullptr);
}

template <BinaryOpCode opCode> void StringTestEwBinarySca(Umbra_t lhs, Umbra_t rhs, int64_t exp) {
    EwBinarySca<opCode, int64_t, Umbra_t, Umbra_t>::apply(lhs, rhs, nullptr);
    ewBinarySca<int64_t, Umbra_t, Umbra_t>(opCode, lhs, rhs, nullptr);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("ReadCsv"), TAG_IO, (DenseMatrix), (PARTIAL_STRING_VALUE_TYPES)) {
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

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("eq- Mat"), TAG_KERNELS, (DenseMatrix), (PARTIAL_STRING_VALUE_TYPES)) {
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
    StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::EQ, m1, m2);

    REQUIRE(m1->getNumRows() == numRows);
    REQUIRE(m1->getNumCols() == numCols);

    DataObjectFactory::destroy(m1);
    DataObjectFactory::destroy(m2);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("eq - Sca"), TAG_IO, (DenseMatrix), (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    size_t numRows = 5000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    for (size_t i = 0; i < 1000; ++i) {
        for (size_t r = 0; r < numRows - 1; ++r) {
            for (size_t r2 = 0; r < numRows - 1; ++r)
                StringTestEwBinarySca<BinaryOpCode::LT>(m->get(r, 2), m->get(r2, 2), 0);
        }
    }

    DataObjectFactory::destroy(m);
}