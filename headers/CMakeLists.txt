add_library(headers INTERFACE)

# target_include_directories(headers INTERFACE
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     $<INSTALL_INTERFACE:include>
# )

target_sources(headers PUBLIC
FILE_SET HEADERS
BASE_DIRS include
FILES
    include/dse/math/color.h
    include/dse/math/constants.h
    include/dse/math/mat.h
    include/dse/math/qmath.h
    include/dse/math/vec.h
    include/dse/math/vmath.h
    include/dse/notifier/notifier.h
    include/dse/util/PoolAllocator.h
    include/dse/util/ProxyContainer.h
    include/dse/util/ProxyIterator.h
    include/dse/util/TagInvoke.h
    include/dse/util/TaskState.h
    include/dse/util/access.h
    include/dse/util/coroutine.h
    include/dse/util/emptylock.h
    include/dse/util/enum_bitwise.h
    include/dse/util/execution.h
    include/dse/util/functional.h
    include/dse/util/impl_coro/auto_task.h
    include/dse/util/impl_coro/eager_task.h
    include/dse/util/impl_coro/generic.h
    include/dse/util/impl_coro/task.h
    include/dse/util/impl_exec/base.h
    include/dse/util/impl_exec/ensure_started.h
    include/dse/util/impl_exec/start_detached.h
    include/dse/util/impl_exec/then.h
    include/dse/util/impl_exec/wait.h
    include/dse/util/pimpl.h
    include/dse/util/scope_exit.h
    include/dse/util/spinlock.h
    include/dse/util/unlock_guard.h
)

install(TARGETS headers EXPORT headers FILE_SET HEADERS)
install(EXPORT headers NAMESPACE dse:: DESTINATION ./headers)
