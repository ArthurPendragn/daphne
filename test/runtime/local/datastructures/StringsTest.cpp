#include <runtime/local/datagen/GenGivenVals.h>
#include <runtime/local/datastructures/DataObjectFactory.h>
#include <runtime/local/datastructures/DenseMatrix.h>
#include <runtime/local/datastructures/Umbra.h>
#include <runtime/local/datastructures/UmbraNew.h>
#include <runtime/local/datastructures/ValueTypeUtils.h>
#include <runtime/local/io/ReadCsv.h>
#include <runtime/local/kernels/EwBinaryMat.h>
#include <runtime/local/kernels/EwBinarySca.h>
#include <runtime/local/kernels/EwUnaryMat.h>
#include <runtime/local/kernels/Fill.h>
#include <runtime/local/kernels/OneHot.h>
#include <runtime/local/kernels/Reshape.h>
#include <runtime/local/kernels/Reverse.h>
#include <runtime/local/kernels/Transpose.h>

#include <tags.h>

#include <catch.hpp>

#include <cstdint>
#include <random>

#define TEST_NAME(opName) "Strings (" opName ")"
#define PARTIAL_STRING_VALUE_TYPES std::string, Umbra_t, NewUmbra_t
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#define LOOP_SIZE 1000
#define NUM_COLS 5
#define NUM_ROWS 34560
#define TEST_FILE_1 "./test/data/strings/uniform_synthetic_random_strings.csv"

#define DELIM ','

template <class DTArg, class DTRes>
void StringTestEwBinaryMat(BinaryOpCode opCode, const DTArg *lhs, const DTArg *rhs) {
    DTRes *res = nullptr;
    ewBinaryMat<DTRes, DTArg, DTArg>(opCode, res, lhs, rhs, nullptr);
    DataObjectFactory::destroy(res);
}

template <BinaryOpCode opCode> void StringTestEwBinarySca(std::string lhs, std::string rhs, int64_t exp) {
    EwBinarySca<opCode, int64_t, std::string, std::string>::apply(lhs, rhs, nullptr);
}

template <BinaryOpCode opCode> void StringTestEwBinarySca(FixedStr16 lhs, FixedStr16 rhs, int64_t exp) {
    EwBinarySca<opCode, int64_t, FixedStr16, FixedStr16>::apply(lhs, rhs, nullptr);
}

template <BinaryOpCode opCode> void StringTestEwBinarySca(Umbra_t lhs, Umbra_t rhs, int64_t exp) {
    EwBinarySca<opCode, int64_t, Umbra_t, Umbra_t>::apply(lhs, rhs, nullptr);
}

template <BinaryOpCode opCode> void StringTestEwBinarySca(NewUmbra_t lhs, NewUmbra_t rhs, int64_t exp) {
    EwBinarySca<opCode, int64_t, NewUmbra_t, NewUmbra_t>::apply(lhs, rhs, nullptr);
}

template <class DTArg, class DTRes> void StringTestEwUnaryMat(UnaryOpCode opCode, const DTArg *arg) {
    DTRes *res = nullptr;
    ewUnaryMat<DTRes, DTArg>(opCode, res, arg, nullptr);
    DataObjectFactory::destroy(res);
}

template <typename VT> void StringTestConcat(VT lhs, VT rhs) {
    EwBinarySca<BinaryOpCode::CONCAT, VT, VT, VT>::apply(lhs, rhs, nullptr);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Uniform(2-11) - ReadCsv"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    readCsv(m, TEST_FILE_1, NUM_ROWS, NUM_COLS, DELIM);

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Uniform(2-11) - get"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;
    DT *m = nullptr;

    readCsv(m, TEST_FILE_1, NUM_ROWS, NUM_COLS, DELIM);

    SECTION("getNumRows()") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            const size_t numRowsLhs = m->getNumRows();
    }
    SECTION("getNumCols()") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            const size_t numColsLhs = m->getNumCols();
    }
    SECTION("getValues()") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            VT *values = m->getValues();
    }
    SECTION("getNumCols()") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            m->getNumCols();
    }

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Uniform(2-11) - EwBinaryMat"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;
    using DTRes = DenseMatrix<int64_t>;

    DT *m1 = nullptr;
    DT *m2 = nullptr;

    readCsv(m1, TEST_FILE_1, NUM_ROWS, NUM_COLS, DELIM);
    readCsv(m2, TEST_FILE_1, NUM_ROWS, NUM_COLS, DELIM);

    REQUIRE(m1->getNumRows() == NUM_ROWS);
    REQUIRE(m1->getNumCols() == NUM_COLS);

    REQUIRE(m2->getNumRows() == NUM_ROWS);
    REQUIRE(m2->getNumCols() == NUM_COLS);

    SECTION("Test") {
        EwBinaryScaFuncPtr<int64_t, VT, VT> func = getEwBinaryScaFuncPtr<int64_t, VT, VT>(BinaryOpCode::EQ);
        DTRes *res = DataObjectFactory::create<DenseMatrix<int64_t>>(NUM_ROWS, NUM_COLS, false);
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            const VT *valuesLhs = m1->getValues();
            const VT *valuesRhs = m2->getValues();
            int64_t *valuesRes = res->getValues();
            for (size_t r = 0; r < NUM_ROWS; r++) {
                for (size_t c = 0; c < NUM_COLS; c++) {
                    valuesRes[c] = func(valuesLhs[c], valuesRhs[c], nullptr);
                }

                valuesLhs += m1->getRowSkip();
                valuesRhs += m2->getRowSkip();
                valuesRes += res->getRowSkip();
            }
        }
    }

    SECTION("EQ") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::EQ, m1, m2);
    }

    SECTION("NEQ") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::NEQ, m1, m2);
    }

    SECTION("LT") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::LT, m1, m2);
    }

    SECTION("GT") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::GT, m1, m2);
    }

    DataObjectFactory::destroy(m1);
    DataObjectFactory::destroy(m2);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Uniform(2-11) - EwBinarySca"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    readCsv(m, TEST_FILE_1, NUM_ROWS, NUM_COLS, DELIM);

    SECTION("EQ") {
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            for (size_t r = 0; r < NUM_ROWS - 1; ++r) {
                for (size_t r2 = 0; r < NUM_ROWS - 1; ++r)
                    StringTestEwBinarySca<BinaryOpCode::EQ>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }
    }

    SECTION("NEQ") {
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            for (size_t r = 0; r < NUM_ROWS - 1; ++r) {
                for (size_t r2 = 0; r < NUM_ROWS - 1; ++r)
                    StringTestEwBinarySca<BinaryOpCode::NEQ>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }
    }

    SECTION("LT") {
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            for (size_t r = 0; r < NUM_ROWS - 1; ++r) {
                for (size_t r2 = 0; r < NUM_ROWS - 1; ++r)
                    StringTestEwBinarySca<BinaryOpCode::LT>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }
    }

    SECTION("GT") {
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            for (size_t r = 0; r < NUM_ROWS - 1; ++r) {
                for (size_t r2 = 0; r < NUM_ROWS - 1; ++r)
                    StringTestEwBinarySca<BinaryOpCode::GT>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }
    }

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Uniform(2-11) - Operations"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;

    DT *m = nullptr;

    readCsv(m, TEST_FILE_1, NUM_ROWS, NUM_COLS, DELIM);

    SECTION("Upper") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwUnaryMat<DT, DT>(UnaryOpCode::UPPER, m);
    }

    SECTION("Lower") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwUnaryMat<DT, DT>(UnaryOpCode::LOWER, m);
    }

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Uniform(2-11) - Operations2"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;

    DT *m = nullptr;

    readCsv(m, TEST_FILE_1, NUM_ROWS, NUM_COLS, DELIM);

    VT resultConcat;
    SECTION("Concat") {
        for (size_t r = 0; r < NUM_ROWS; r++) {
            resultConcat = ewBinarySca<VT, VT, VT>(BinaryOpCode::CONCAT, resultConcat, m->get(r, 0), nullptr);
        }
    }

    DataObjectFactory::destroy(m);
}

// CAST TESTS
/*
TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Uniform(2-11) - RecodeAndOneHotStrings"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (ALL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;
    using DTRes = DenseMatrix<int64_t>;

    DT *arg = nullptr;
    size_t numRows = 50000;
    size_t numCols = 5;
    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';
    readCsv(arg, filename, numRows, numCols, delim);

    DenseMatrix<int64_t> *info = genGivenVals<DenseMatrix<int64_t>>(1, {0, -1, 0, 0, 0});

    DTRes *oneHotRes = nullptr;
    oneHot(oneHotRes, arg, info, nullptr);

    REQUIRE(oneHotRes->getNumRows() == numRows);

    DataObjectFactory::destroy(arg, arg, oneHotRes);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Uniform(2-11) - Data Generation"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;

    DT *m = nullptr;
    size_t numRows = 50000;
    size_t numCols = 5;
    char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    SECTION("Set") {
        size_t sampleSize = 100;
        DT *sample = DataObjectFactory::create<DT>(sampleSize, numCols, false);

        std::mt19937 rng(42); // fixed seed for reproducibility
        std::uniform_int_distribution<size_t> dist(0, numRows - 1);

        for (size_t k = 0; k < 100; k++) {
            for (size_t i = 0; i < sampleSize; i++) {
                size_t rowIdx = dist(rng);
                for (size_t c = 0; c < numCols; c++) {
                    sample->set(i, c, m->get(rowIdx, c));
                }
            }
        }
        REQUIRE(sample->getNumRows() == sampleSize);
        DataObjectFactory::destroy(sample);
    }

    SECTION("Transpose") {
        DT *res = nullptr;
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            transpose<DT, DT>(res, m, nullptr);
        }
    }
    SECTION("Reverse") {
        DT *res = nullptr;
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            reverse<DT, DT>(res, m, nullptr);
        }
    }
    SECTION("Reshape") {
        DT *res = nullptr;
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            reshape<DT, DT>(res, m, 25000, 10, nullptr);
        }
    }

    SECTION("Fill Long") {
        VT arg = VT("abcdefghijklmnopqrstuvwxyz");
        DenseMatrix<VT> *res = nullptr;
        fill(res, arg, numRows, numCols, nullptr);
    }

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Skewed(2-100) - ReadCsv"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    size_t numRows = 50000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/skewed_synthetic_random_strings-2-100.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    REQUIRE(m->getNumRows() == numRows);
    REQUIRE(m->getNumCols() == numCols);

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Skewed(2-100) - EwBinaryMat"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using DTRes = DenseMatrix<int64_t>;

    DT *m1 = nullptr;
    DT *m2 = nullptr;

    size_t numRows = 50000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/skewed_synthetic_random_strings-2-100.csv";
    char delim = ',';

    readCsv(m1, filename, numRows, numCols, delim);
    readCsv(m2, filename, numRows, numCols, delim);

    SECTION("EQ") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::EQ, m1, m2);
    }

    SECTION("NEQ") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::NEQ, m1, m2);
    }

    SECTION("LT") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::LT, m1, m2);
    }

    SECTION("GT") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::GT, m1, m2);
    }

    REQUIRE(m1->getNumRows() == numRows);
    REQUIRE(m1->getNumCols() == numCols);

    DataObjectFactory::destroy(m1);
    DataObjectFactory::destroy(m2);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Skewed(2-100) - EwBinarySca"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    DT *m = nullptr;

    size_t numRows = 50000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/skewed_synthetic_random_strings-2-100.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    SECTION("EQ") {
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            for (size_t r = 0; r < numRows - 1; ++r) {
                for (size_t r2 = 0; r < numRows - 1; ++r)
                    StringTestEwBinarySca<BinaryOpCode::EQ>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }
    }

    SECTION("NEQ") {
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            for (size_t r = 0; r < numRows - 1; ++r) {
                for (size_t r2 = 0; r < numRows - 1; ++r)
                    StringTestEwBinarySca<BinaryOpCode::NEQ>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }
    }

    SECTION("LT") {
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            for (size_t r = 0; r < numRows - 1; ++r) {
                for (size_t r2 = 0; r < numRows - 1; ++r)
                    StringTestEwBinarySca<BinaryOpCode::LT>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }
    }

    SECTION("GT") {
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            for (size_t r = 0; r < numRows - 1; ++r) {
                for (size_t r2 = 0; r < numRows - 1; ++r)
                    StringTestEwBinarySca<BinaryOpCode::GT>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }
    }

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Skewed(2-100) - Operations"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;

    DT *m = nullptr;

    size_t numRows = 50000;
    size_t numCols = 5;

    char filename[] = "./test/data/strings/skewed_synthetic_random_strings-2-100.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    SECTION("Upper") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwUnaryMat<DT, DT>(UnaryOpCode::UPPER, m);
    }

    SECTION("Lower") {
        for (size_t i = 0; i < LOOP_SIZE; i++)
            StringTestEwUnaryMat<DT, DT>(UnaryOpCode::LOWER, m);
    }

    DataObjectFactory::destroy(m);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Skewed(2-100) - Operations2"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;

    DT *m = nullptr;
    size_t numRows = 50000;
    size_t numCols = 5;
    char filename[] = "./test/data/strings/skewed_synthetic_random_strings-2-100.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    VT resultConcat;
    SECTION("Concat") {

        for (size_t r = 0; r < numRows; r++) {
            resultConcat = ewBinarySca<VT, VT, VT>(BinaryOpCode::CONCAT, resultConcat, m->get(r, 0), nullptr);
        }
    }

    DataObjectFactory::destroy(m);
}

// CAST TESTS

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Skewed(2-100) - RecodeAndOneHotStrings"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;
    using DTRes = DenseMatrix<int64_t>;

    DT *arg = nullptr;
    size_t numRows = 50000;
    size_t numCols = 5;
    char filename[] = "./test/data/strings/skewed_synthetic_random_strings-2-100.csv";
    char delim = ',';
    readCsv(arg, filename, numRows, numCols, delim);

    DenseMatrix<int64_t> *info = genGivenVals<DenseMatrix<int64_t>>(1, {0, -1, 0, 0, 0});

    DTRes *oneHotRes = nullptr;
    oneHot(oneHotRes, arg, info, nullptr);

    REQUIRE(oneHotRes->getNumRows() == numRows);

    DataObjectFactory::destroy(arg, arg, oneHotRes);
}

TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Skewed(2-100) - Data Generation"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;

    DT *m = nullptr;
    size_t numRows = 50000;
    size_t numCols = 5;
    char filename[] = "./test/data/strings/skewed_synthetic_random_strings-2-100.csv";
    char delim = ',';

    readCsv(m, filename, numRows, numCols, delim);

    SECTION("Set") {
        size_t sampleSize = 100;
        DT *sample = DataObjectFactory::create<DT>(sampleSize, numCols, false);

        std::mt19937 rng(42);
        std::uniform_int_distribution<size_t> dist(0, numRows - 1);

        for (size_t k = 0; k < 100; k++) {
            for (size_t i = 0; i < sampleSize; i++) {
                size_t rowIdx = dist(rng);
                for (size_t c = 0; c < numCols; c++) {
                    sample->set(i, c, m->get(rowIdx, c));
                }
            }
        }
        REQUIRE(sample->getNumRows() == sampleSize);
        DataObjectFactory::destroy(sample);
    }

    SECTION("Transpose") {
        DT *res = nullptr;
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            transpose<DT, DT>(res, m, nullptr);
        }
    }
    SECTION("Reverse") {
        DT *res = nullptr;
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            reverse<DT, DT>(res, m, nullptr);
        }
    }
    SECTION("Reshape") {
        DT *res = nullptr;
        for (size_t i = 0; i < LOOP_SIZE; i++) {
            reshape<DT, DT>(res, m, 25000, 10, nullptr);
        }
    }
    SECTION("Fill") {
        DenseMatrix<VT> *res = nullptr;
        VT arg = VT("abc");

        fill(res, arg, numRows, numCols, nullptr);
    }

    SECTION("Fill Long") {
        DenseMatrix<VT> *res = nullptr;
        VT arg = VT("abcdefghijklmnopqrstuvwxyz");

        fill(res, arg, numRows, numCols, nullptr);
    }

    DataObjectFactory::destroy(m);
}
*/