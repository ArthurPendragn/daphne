#include <runtime/local/datagen/GenGivenVals.h>
#include <runtime/local/datastructures/DataObjectFactory.h>
#include <runtime/local/datastructures/DenseMatrix.h>
#include <runtime/local/datastructures/ValueTypeUtils.h>
#include <runtime/local/kernels/EwBinarySca.h>

#include <tags.h>

#include <catch.hpp>

#include <cstdint>

TEMPLATE_PRODUCT_TEST_CASE("String Operations in DenseMatrix", TAG_IO, (DenseMatrix), (ALL_STRING_VALUE_TYPES)) {
    using ValueType = TestType;

    SECTION("Read CSV and Check Dimensions") {
        DenseMatrix<ValueType> *m = nullptr;

        size_t numRows = 5000;
        size_t numCols = 5;

        char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
        char delim = ',';

        readCsv(m, filename, numRows, numCols, delim);

        REQUIRE(m->getNumRows() == numRows);
        REQUIRE(m->getNumCols() == numCols);

        DataObjectFactory::destroy(m);
    }

    SECTION("Element-wise Comparisons and Simulated Join/Sort") {
        using VT = TestType;

        DenseMatrix<VT> *m = nullptr;

        size_t numRows = 5000;
        size_t numCols = 5;
        char filename[] = "test/data/strings/uniform_synthetic_random_strings.csv";
        char delim = ',';

        readCsv(m, filename, numRows, numCols, delim);

        // Simulate exact match join on the first column
        for (size_t r = 0; r < numRows - 1; ++r) {
            CHECK(checkEwBinarySca<BinaryOpCode::EQ>(m->get(r, 0), m->get(r + 1, 0), 0)); // Expect no duplicate rows
        }

        // Inequality checks
        for (size_t r = 0; r < numRows; ++r) {
            for (size_t c1 = 0; c1 < numCols; ++c1) {
                for (size_t c2 = 0; c2 < numCols; ++c2) {
                    if (c1 != c2) {
                        CHECK(checkEwBinarySca<BinaryOpCode::NEQ>(m->get(r, c1), m->get(r, c2), 1));
                    }
                }
            }
        }

        // Simulate sorting by the first column
        for (size_t r = 0; r < numRows - 1; ++r) {
            CHECK(checkEwBinarySca<BinaryOpCode::LE>(m->get(r, 0), m->get(r + 1, 0), 1));
        }

        // Simulate a "join" by concatenating the first two columns for each row
        DenseMatrix<VT> *joinedMatrix = DataObjectFactory::create<DenseMatrix<VT>>(numRows, 1, false);
        for (size_t r = 0; r < numRows; ++r) {
            VT joinedValue = m->get(r, 0) + VT(" ") + m->get(r, 1);
            joinedMatrix->set(r, 0, joinedValue);
        }

        CHECK(joinedMatrix->get(0, 0) == m->get(0, 0) + VT(" ") + m->get(0, 1));
        CHECK(joinedMatrix->get(numRows - 1, 0) == m->get(numRows - 1, 0) + VT(" ") + m->get(numRows - 1, 1));

        DataObjectFactory::destroy(m);
        DataObjectFactory::destroy(joinedMatrix);
    }

    SECTION("Convert All Strings to Uppercase") {
        DenseMatrix<ValueType> *m = nullptr;

        size_t numRows = 9;
        size_t numCols = 3;

        char filename[] = "./test/data/strings/uniform_synthetic_random_strings.csv";
        char delim = ',';

        readCsv(m, filename, numRows, numCols, delim);

        for (size_t r = 0; r < numRows; ++r) {
            for (size_t c = 0; c < numCols; ++c) {
                ValueType val = m->get(r, c);
                std::transform(val.begin(), val.end(), val.begin(), ::toupper);
                m->set(r, c, val);
            }
        }

        CHECK(m->get(0, 0) == ValueType("APPLE, ORANGE"));
        CHECK(m->get(1, 0) == ValueType("DOG, CAT"));

        DataObjectFactory::destroy(m);
    }

    TEMPLATE_PRODUCT_TEST_CASE(TEST_NAME("Upper, string data from sorted dataset"), TAG_KERNELS, (DenseMatrix),
                               (ALL_STRING_VALUE_TYPES)) {
        using DT = TestType;
        using VT = typename DT::VT;

        DenseMatrix<VT> *arg = nullptr;

        size_t numRows = 5000;
        size_t numCols = 5;
        char filename[] = "test/data/strings/uniform_synthetic_random_strings.csv";
        char delim = ',';

        readCsv(arg, filename, numRows, numCols, delim);

        DenseMatrix<VT> *exp = DataObjectFactory::create<DenseMatrix<VT>>(numRows, numCols, false);

        for (size_t r = 0; r < numRows; ++r) {
            for (size_t c = 0; c < numCols; ++c) {
                VT upperString = arg->get(r, c);
                std::transform(upperString.begin(), upperString.end(), upperString.begin(), ::toupper);
                exp->set(r, c, upperString);
            }
        }

        checkEwUnaryMat(UnaryOpCode::UPPER, arg, exp);

        DataObjectFactory::destroy(arg, exp);
    }

    SECTION("Large Number of Fill Operations") {
        const size_t numRows = 1000;
        const size_t numCols = 1000;

        DenseMatrix<ValueType> *m = DataObjectFactory::create<DenseMatrix<ValueType>>(numRows, numCols, false);

        ValueType filler = "TestValue";
        for (size_t r = 0; r < numRows; ++r) {
            for (size_t c = 0; c < numCols; ++c) {
                m->set(r, c, filler);
            }
        }

        // Verify a few cells
        CHECK(m->get(0, 0) == filler);
        CHECK(m->get(numRows - 1, numCols - 1) == filler);

        DataObjectFactory::destroy(m);
    }
}