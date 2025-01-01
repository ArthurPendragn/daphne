#include <runtime/local/datastructures/DataObjectFactory.h>
#include <runtime/local/datastructures/DenseMatrix.h>
#include <runtime/local/datastructures/ValueTypeUtils.h>
#include <runtime/local/io/File.h>
#include <runtime/local/io/ReadCsv.h>
#include <runtime/local/kernels/EwBinarySca.h>

#include <tags.h>

#include <catch.hpp>

#include <cstdint>

TEMPLATE_PRODUCT_TEST_CASE("Check Dimensions of CSV", TAG_IO, (DenseMatrix), (std::string)) {
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