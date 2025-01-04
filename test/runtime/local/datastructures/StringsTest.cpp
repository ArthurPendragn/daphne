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
#include <runtime/local/kernels/OneHot.h>

#include <tags.h>

#include <catch.hpp>

#include <cstdint>
#include <random>

// Define macros and helper functions as in the original file
#define TEST_NAME(opName) "Strings (" opName ")"
#define PARTIAL_STRING_VALUE_TYPES std::string, Umbra_t, NewUmbra_t

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

// ======================= Test Case 1 =======================
TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("from a uniform distribution with lenght(2-11)"), TAG_DATASTRUCTURES,
                           (DenseMatrix), (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;
    using DTRes = DenseMatrix<int64_t>;
    const size_t numRows = 50000;
    const size_t numCols = 5;
    const char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
    const char delim = ',';

    SECTION("ReadCsv") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        REQUIRE(m->getNumRows() == numRows);
        REQUIRE(m->getNumCols() == numCols);

        DataObjectFactory::destroy(m);
    }

    SECTION("eq - Mat") {
        DT *m1 = nullptr;
        DT *m2 = nullptr;

        readCsv(m1, filename, numRows, numCols, delim);
        readCsv(m2, filename, numRows, numCols, delim);

        for (size_t i = 0; i < 1000; i++) {
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::EQ, m1, m2);
        }

        REQUIRE(m1->getNumRows() == numRows);
        REQUIRE(m1->getNumCols() == numCols);

        DataObjectFactory::destroy(m1);
        DataObjectFactory::destroy(m2);
    }

    SECTION("eq - Sca") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        for (size_t r = 0; r < numRows - 1; ++r) {
            for (size_t r2 = 0; r2 < numRows - 1; ++r2) {
                StringTestEwBinarySca<BinaryOpCode::EQ>(m->get(r, 0), m->get(r2, 0), 0);
            }
        }

        for (size_t r = 0; r < numRows - 1; ++r) {
            for (size_t r2 = 0; r2 < numRows - 1; ++r2) {
                StringTestEwBinarySca<BinaryOpCode::LT>(m->get(r, 2), m->get(r2, 2), 0);
            }
        }

        REQUIRE(m->getNumRows() == numRows);
        REQUIRE(m->getNumCols() == numCols);

        DataObjectFactory::destroy(m);
    }

    SECTION("Upper") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        for (size_t i = 0; i < 100; i++) {
            StringTestEwUnaryMat<DT, DT>(UnaryOpCode::UPPER, m);
        }

        DataObjectFactory::destroy(m);
    }

    SECTION("ConcatenateAllRows") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        VT resultConcat;
        for (size_t r = 0; r < numRows; r++) {
            resultConcat = EwBinarySca<BinaryOpCode::CONCAT, VT, VT, VT>::apply(resultConcat, m->get(r, 0), nullptr);
        }

        DataObjectFactory::destroy(m);
    }

    SECTION("RecodeAndOneHotStrings") {
        DT *arg = nullptr;
        readCsv(arg, filename, numRows, numCols, delim);

        DenseMatrix<int64_t> *info = genGivenVals<DenseMatrix<int64_t>>(1, {0, -1, 0, 0, 0});

        DTRes *oneHotRes = nullptr;
        oneHot(oneHotRes, arg, info, nullptr);

        REQUIRE(oneHotRes->getNumRows() == numRows);

        DataObjectFactory::destroy(arg);
        DataObjectFactory::destroy(oneHotRes);
    }

    SECTION("SampleStringData") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

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

        DataObjectFactory::destroy(m);
        DataObjectFactory::destroy(sample);
    }
}

// ======================= Test Case 2 =======================
TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("from a skewd distribution with lenght(2-100)"), TAG_DATASTRUCTURES, (DenseMatrix),
                           (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;
    using DTRes = DenseMatrix<int64_t>;
    const size_t numRows = 50000;
    const size_t numCols = 5;
    const char filename[] = " ./test/data/strings/skewed_synthetic_random_strings-2-100.csv";
    const char delim = ',';

    SECTION("ReadCsv") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        REQUIRE(m->getNumRows() == numRows);
        REQUIRE(m->getNumCols() == numCols);

        DataObjectFactory::destroy(m);
    }

    SECTION("eq - Mat") {
        DT *m1 = nullptr;
        DT *m2 = nullptr;

        readCsv(m1, filename, numRows, numCols, delim);
        readCsv(m2, filename, numRows, numCols, delim);

        for (size_t i = 0; i < 1000; i++) {
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::EQ, m1, m2);
        }

        REQUIRE(m1->getNumRows() == numRows);
        REQUIRE(m1->getNumCols() == numCols);

        DataObjectFactory::destroy(m1);
        DataObjectFactory::destroy(m2);
    }

    SECTION("eq - Sca") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        for (size_t i = 0; i < 100; i++) {
            for (size_t r = 0; r < numRows - 1; ++r) {
                for (size_t r2 = 0; r2 < numRows - 1; ++r2) {
                    StringTestEwBinarySca<BinaryOpCode::EQ>(m->get(r, 0), m->get(r2, 0), 0);
                }
            }
        }

        for (size_t i = 0; i < 100; i++) {
            for (size_t r = 0; r < numRows - 1; ++r) {
                for (size_t r2 = 0; r2 < numRows - 1; ++r2) {
                    StringTestEwBinarySca<BinaryOpCode::LT>(m->get(r, 2), m->get(r2, 2), 0);
                }
            }
        }

        DataObjectFactory::destroy(m);
    }

    SECTION("Upper") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        for (size_t i = 0; i < 100; i++) {
            StringTestEwUnaryMat<DT, DT>(UnaryOpCode::UPPER, m);
        }

        DataObjectFactory::destroy(m);
    }

    SECTION("ConcatenateAllRows") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        VT resultConcat;
        for (size_t r = 0; r < numRows; r++) {
            resultConcat = EwBinarySca<BinaryOpCode::CONCAT, VT, VT, VT>::apply(resultConcat, m->get(r, 0), nullptr);
        }

        DataObjectFactory::destroy(m);
    }

    SECTION("RecodeAndOneHotStrings") {
        DT *arg = nullptr;
        readCsv(arg, filename, numRows, numCols, delim);

        DenseMatrix<int64_t> *info = genGivenVals<DenseMatrix<int64_t>>(1, {0, -1, 0, 0, 0});

        DTRes *oneHotRes = nullptr;
        oneHot(oneHotRes, arg, info, nullptr);

        REQUIRE(oneHotRes->getNumRows() == numRows);

        DataObjectFactory::destroy(arg);
        DataObjectFactory::destroy(oneHotRes);
    }

    SECTION("SampleStringData") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

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

        DataObjectFactory::destroy(m);
        DataObjectFactory::destroy(sample);
    }
}

// ======================= Test Case 3 =======================
TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("from a skewd distribution with lenght(200-1000)"), TAG_DATASTRUCTURES,
                           (DenseMatrix), (PARTIAL_STRING_VALUE_TYPES)) {
    using DT = TestType;
    using VT = typename DT::VT;
    using DTRes = DenseMatrix<int64_t>;
    const size_t numRows = 50000;
    const size_t numCols = 5;
    const char filename[] = " ./test/data/strings/skewed_synthetic_random_strings-200-1000.csv";
    const char delim = ',';

    SECTION("ReadCsv") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        REQUIRE(m->getNumRows() == numRows);
        REQUIRE(m->getNumCols() == numCols);

        DataObjectFactory::destroy(m);
    }

    SECTION("eq - Mat") {
        DT *m1 = nullptr;
        DT *m2 = nullptr;

        readCsv(m1, filename, numRows, numCols, delim);
        readCsv(m2, filename, numRows, numCols, delim);

        for (size_t i = 0; i < 1000; i++) {
            StringTestEwBinaryMat<DT, DTRes>(BinaryOpCode::EQ, m1, m2);
        }

        REQUIRE(m1->getNumRows() == numRows);
        REQUIRE(m1->getNumCols() == numCols);

        DataObjectFactory::destroy(m1);
        DataObjectFactory::destroy(m2);
    }

    SECTION("eq - Sca") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        for (size_t i = 0; i < 100; i++) {
            for (size_t r = 0; r < numRows - 1; ++r) {
                for (size_t r2 = 0; r2 < numRows - 1; ++r2) {
                    StringTestEwBinarySca<BinaryOpCode::EQ>(m->get(r, 0), m->get(r2, 0), 0);
                }
            }
        }

        for (size_t i = 0; i < 100; i++) {
            for (size_t r = 0; r < numRows - 1; ++r) {
                for (size_t r2 = 0; r2 < numRows - 1; ++r2) {
                    StringTestEwBinarySca<BinaryOpCode::LT>(m->get(r, 2), m->get(r2, 2), 0);
                }
            }
        }

        DataObjectFactory::destroy(m);
    }

    SECTION("Upper") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        for (size_t i = 0; i < 100; i++) {
            StringTestEwUnaryMat<DT, DT>(UnaryOpCode::UPPER, m);
        }

        DataObjectFactory::destroy(m);
    }

    SECTION("ConcatenateAllRows") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

        VT resultConcat;
        for (size_t r = 0; r < numRows; r++) {
            resultConcat = EwBinarySca<BinaryOpCode::CONCAT, VT, VT, VT>::apply(resultConcat, m->get(r, 0), nullptr);
        }

        DataObjectFactory::destroy(m);
    }

    SECTION("RecodeAndOneHotStrings") {
        DT *arg = nullptr;
        readCsv(arg, filename, numRows, numCols, delim);

        DenseMatrix<int64_t> *info = genGivenVals<DenseMatrix<int64_t>>(1, {0, -1, 0, 0, 0});

        DTRes *oneHotRes = nullptr;
        oneHot(oneHotRes, arg, info, nullptr);

        REQUIRE(oneHotRes->getNumRows() == numRows);

        DataObjectFactory::destroy(arg);
        DataObjectFactory::destroy(oneHotRes);
    }

    SECTION("SampleStringData") {
        DT *m = nullptr;
        readCsv(m, filename, numRows, numCols, delim);

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

        DataObjectFactory::destroy(m);
        DataObjectFactory::destroy(sample);
    }
}
