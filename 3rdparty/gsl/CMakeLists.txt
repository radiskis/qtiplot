
project(gsl C)

set(GSL_DIRS
    blas block bspline bst cdf cheb combination complex const deriv dht diff
    eigen err fft filter fit histogram ieee-utils integration interpolation
    linalg matrix min monte movstat multifit multifit_nlinear multilarge
    multilarge_nlinear multimin multiroots multiset ntuple ode-initval
    ode-initval2 permutation poly qrng randist rng roots rstat siman sort
    spblas specfunc splinalg spmatrix statistics sum sys vector wavelet
)

set(GSL_SOURCES "")

foreach(dir ${GSL_DIRS})
    file(GLOB dir_sources "${dir}/*.c")
    
    # List of files to exclude (templates/headers included by other .c files)
    set(GSL_EXCLUDES
        Qn_source.c Sn_source.c absdev_source.c append.c apply_givens.c beta_inc.c bitreverse.c block_source.c
        c_init.c c_main.c c_pass_2.c c_pass_3.c c_pass_4.c c_pass_5.c c_pass_6.c c_pass_7.c c_pass_n.c c_radix2.c
        cheb_eval.c cheb_eval_mode.c cholesky_common.c common.c companion.c compare_source.c compress_source.c control_utils.c
        copy_complex_source.c copy_source.c covariance_source.c cquad_const.c demo_fn.c deque.c dft_source.c
        directional_minimize.c dogleg.c endian.c enorm.c err.c factorize.c file_source.c find.c find2d.c
        fp-aix.c fp-freebsd.c fp-gnuc99.c fp-gnum68k.c fp-gnuppc.c fp-gnusparc.c fp-gnux86.c fp-hpux.c fp-hpux11.c
        fp-irix.c fp-netbsd.c fp-openbsd.c fp-os2emx.c fp-solaris.c fp-sunos4.c fp-tru64.c fp-unknown.c
        fprintf_source.c fwrite_source.c gastwirth_source.c getset_complex_source.c getset_source.c hc_init.c
        hc_main.c hc_pass_2.c hc_pass_3.c hc_pass_4.c hc_pass_5.c hc_pass_n.c hc_radix2.c hc_unpack.c
        init_source.c initialise.c kurtosis_source.c lag1_source.c legendre_source.c linear_common.c linear_minimize.c
        linear_wrapper.c lmiterate.c lmmisc.c lmpar.c lmset.c lmutil.c mad_source.c mean_source.c median_source.c
        minmax_source.c modnewton1.c nielsen.c oper_complex_source.c oper_source.c p_variance_source.c permute_source.c
        positivity.c prop_source.c ptsort.c qc25c.c qc25f.c qc25s.c qelg.c qpsrt.c qpsrt2.c qrsolv.c qrstep.c
        quantiles_source.c real_init.c real_main.c real_pass_2.c real_pass_3.c real_pass_4.c real_pass_5.c
        real_pass_n.c real_radix2.c real_unpack.c reim_source.c reset.c ringbuf.c rksubs.c rowcol_source.c schrage.c
        select_source.c set_initial.c signals_source.c skew_source.c sortvec_source.c sortvecind_source.c standardize.c
        step_utils.c submatrix_source.c subset_source.c subsetind_source.c subvector_source.c svdstep.c
        swap_complex_source.c swap_source.c trmean_source.c urand.c util.c variance_source.c view_source.c
        wabsdev_source.c wkurtosis_source.c wmean_source.c wskew_source.c wvariance_source.c
    )

    foreach(src ${dir_sources})
        get_filename_component(fname ${src} NAME)
        file(RELATIVE_PATH rel_src ${CMAKE_CURRENT_SOURCE_DIR} ${src})
        
        # Special case: exclude all ieee-utils/fp-*.c except fp-unknown.c
        set(SKIP_FILE FALSE)
        if(src MATCHES "ieee-utils/fp-.*\\.c" AND NOT fname STREQUAL "fp-unknown.c")
            set(SKIP_FILE TRUE)
        endif()

        # Specific exclusions with folder paths
        if(rel_src STREQUAL "poly/qr.c" OR rel_src STREQUAL "poly/balance.c" OR rel_src STREQUAL "cblas/hypot.c")
            set(SKIP_FILE TRUE)
        endif()

        if(NOT src MATCHES "test" AND NOT fname IN_LIST GSL_EXCLUDES AND NOT SKIP_FILE)
            list(APPEND GSL_SOURCES ${src})
            # Ensure unique object names to avoid archive collisions
            string(REPLACE "/" "_" obj_name "${rel_src}")
            set_source_files_properties(${src} PROPERTIES OBJECT_OUTPUT_NAME "${obj_name}")
        else()
             message(STATUS "Skipping ${src}")
        endif()
    endforeach()
endforeach()

add_library(gsl STATIC ${GSL_SOURCES})

target_include_directories(gsl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

# gslcblas library
file(GLOB CBLAS_SOURCES "cblas/*.c")
set(CBLAS_EXCLUDES "") # Add if needed
set(CBLAS_FINAL_SOURCES "")
foreach(src ${CBLAS_SOURCES})
    if(NOT src MATCHES "test")
        list(APPEND CBLAS_FINAL_SOURCES ${src})
    endif()
endforeach()

add_library(gslcblas STATIC ${CBLAS_FINAL_SOURCES})
target_include_directories(gslcblas PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(MSVC)
    target_compile_options(gsl PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/config.h" "/FI${CMAKE_CURRENT_SOURCE_DIR}/gsl/gsl_math.h" "/FI${CMAKE_CURRENT_SOURCE_DIR}/gsl/gsl_complex.h")
    target_compile_options(gslcblas PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/config.h" "/FI${CMAKE_CURRENT_SOURCE_DIR}/gsl/gsl_math.h" "/FI${CMAKE_CURRENT_SOURCE_DIR}/gsl/gsl_complex.h")
endif()
