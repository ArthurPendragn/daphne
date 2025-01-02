#include <runtime/local/datastructures/DataObjectFactory.h>
#include <runtime/local/datastructures/DenseMatrix.h>
#include <runtime/local/datastructures/Umbra.h>
#include <runtime/local/datastructures/ValueTypeUtils.h>
#include <runtime/local/io/ReadCsv.h>
#include <runtime/local/kernels/EwBinaryMat.h>
#include <runtime/local/kernels/EwBinarySca.h>
#include <runtime/local/kernels/EwUnaryMat.h>

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

template <BinaryOpCode opCode> void StringTestEwBinarySca(FixedStr16 lhs, FixedStr16 rhs, int64_t exp) {
    EwBinarySca<opCode, int64_t, FixedStr16, FixedStr16>::apply(lhs, rhs, nullptr);
    ewBinarySca<int64_t, FixedStr16, FixedStr16>(opCode, lhs, rhs, nullptr);
}

template <BinaryOpCode opCode> void StringTestEwBinarySca(Umbra_t lhs, Umbra_t rhs, int64_t exp) {
    EwBinarySca<opCode, int64_t, Umbra_t, Umbra_t>::apply(lhs, rhs, nullptr);
    ewBinarySca<int64_t, Umbra_t, Umbra_t>(opCode, lhs, rhs, nullptr);
}

template <typename DTRes, typename DTArg> void StringTestEwUnaryMat(UnaryOpCode opCode, const DTArg *arg) {
    DTRes *res = nullptr;
    ewUnaryMat<DTRes, DTArg>(opCode, res, arg, nullptr);
    DataObjectFactory::destroy(res);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("ReadCsv"), TAG_DATASTRUCTURES, (DenseMatrix), (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    size_t numRows = 50000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    REQUIRE(m->getNumRows() == numRows);
    REQUIRE(m->getNumCols() == numCols);

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("eq- Mat"), TAG_DATASTRUCTURES, (DenseMatrix), (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using DTRes = DenseMatrix<int64_t>;

    DT *m1 = nullptr;
    DT *m2 = nullptr;

    size_t numRows = 50000;
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

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("eq - Sca"), TAG_DATASTRUCTURES, (DenseMatrix), (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    size_t numRows = 50000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    for (size_t r = 0; r < numRows - 1; ++r) {
        for (size_t r2 = 0; r < numRows - 1; ++r)
            StringTestEwBinarySca<BinaryOpCode::EQ>(m->get(r, 0), m->get(r2, 0), 0);
    }

    for (size_t r = 0; r < numRows - 1; ++r) {
        for (size_t r2 = 0; r < numRows - 1; ++r)
            StringTestEwBinarySca<BinaryOpCode::LT>(m->get(r, 2), m->get(r2, 2), 0);
    }

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE("Convert Strings to Uppercase", TAG_DATASTRUCTURES, (DenseMatrix),
                           (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    size_t numRows = 50000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    StringTestEwUnaryMat(m);

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE("Large Number of Fill Operations", TAG_DATASTRUCTURES, (DenseMatrix),
                           (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;
    const size_t numRows = 10000;
    const size_t numCols = 100;

    DenseMatrix<DT> *m = DataObjectFactory::create<DenseMatrix<DT>>(numRows, numCols, false);

    DT filler = "123456789012"; // Length 12
    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            m->set(r, c, filler);
        }
    }

    DataObjectFactory::destroy(m);
}