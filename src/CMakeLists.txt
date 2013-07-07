# example of configure time generate header

include_directories (./ ${SOURCE_DIR} ${SOURCE_DIR}/utils ${SOURCE_DIR}/math)

message(STATUS ${SOURCE_DIR})

set (lgdpj_SRC collections.cpp
    featurespace.cpp
    extractor.cpp
    options.cpp
    decoder1o.cpp
    decoder2o.cpp
    parser.cpp
    lgdpj.cpp)

add_executable(lgdpj ${lgdpj_SRC})

