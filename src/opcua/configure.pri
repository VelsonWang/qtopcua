defineTest(qtConfLibrary_uacpp) {
    input = $$eval($${2}.alias)
    prefix = $$eval(config.input.$${input}.prefix)

    # The Windows SDK ships its dependencies and locates the libraries outside of lib
    win32 {
        lessThan(QMAKE_MSC_VER, 1900) {
            qtLog("UaCpp SDK requires at least Visual Studio 2015")
            return(false)
        }

        !isEmpty(prefix) {
            config.input.$${input}.libdir += \
                $${prefix}/third-party/win32/vs2015/openssl/out32dll \
                $${prefix}/third-party/win32/vs2015/libxml2/out32dll
        } else {
            qtLog("No UACPP_PREFIX specified; relying on user-provided library path.")
        }
    }

    inc = $$eval(config.input.$${input}.incdir)
    isEmpty(inc):!isEmpty(prefix): \
        inc = $${prefix}/include
    !isEmpty(inc) {
        config.input.$${input}.incdir += \
            $$inc/uabase $$inc/uaclient $$inc/uastack $$inc/uapki
    } else {
        qtLog("No UACPP_PREFIX and no UACPP_INCDIR specified; relying on global include paths.")
    }

    !qtConfLibrary_inline($$1, $$2): \
        return(false)
    return(true)
}
