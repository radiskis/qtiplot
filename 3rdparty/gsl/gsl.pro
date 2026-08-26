TEMPLATE = lib
CONFIG += staticlib object_with_source
TARGET = gsl
DESTDIR = lib
INCLUDEPATH += . ..
INCLUDEPATH += . ..
INCLUDEPATH += . ..
INCLUDEPATH += . ..
INCLUDEPATH += . ..


DIRS = \
    blas \
    block \
    bspline \
    bst \
    cdf \
    cheb \
    combination \
    complex \
    const \
    deriv \
    dht \
    diff \
    eigen \
    err \
    fft \
    filter \
    fit \
    histogram \
    ieee-utils \
    integration \
    interpolation \
    linalg \
    matrix \
    min \
    monte \
    movstat \
    multifit \
    multifit_nlinear \
    multilarge \
    multilarge_nlinear \
    multimin \
    multiroots \
    multiset \
    ntuple \
    ode-initval \
    ode-initval2 \
    permutation \
    poly \
    qrng \
    randist \
    rng \
    roots \
    rstat \
    siman \
    sort \
    spblas \
    specfunc \
    splinalg \
    spmatrix \
    statistics \
    sum \
    sys \
    vector \
    wavelet

for(dir, DIRS) {
    SOURCES += $$files($$dir/*.c)
    SOURCES -= $$files($$dir/test*.c)
    SOURCES -= $$files($$dir/*test.c)
    SOURCES -= $$files($$dir/*_source.c)
}
