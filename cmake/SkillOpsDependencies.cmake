include_guard(GLOBAL)

function(skillops_try_microservice_kit package_name target_name out_found)
    find_package(${package_name} QUIET)
    if(TARGET ${target_name})
        add_library(skillops::microservice_kit INTERFACE IMPORTED GLOBAL)
        target_link_libraries(skillops::microservice_kit INTERFACE ${target_name})
        set(SKILLOPS_MICROSERVICE_KIT_PACKAGE "${package_name}" CACHE STRING "Resolved C++ microservice kit package")
        set(SKILLOPS_MICROSERVICE_KIT_TARGET "${target_name}" CACHE STRING "Resolved C++ microservice kit target")
        message(STATUS "SkillOps using C++ microservice kit: ${package_name} (${target_name})")
        set(${out_found} TRUE PARENT_SCOPE)
    else()
        set(${out_found} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(skillops_find_microservice_kit)
    skillops_try_microservice_kit(microkit microkit::microkit _found)
    if(_found)
        return()
    endif()

    skillops_try_microservice_kit(tew_scaffold tew_scaffold::tew_scaffold _found)
    if(_found)
        return()
    endif()

    skillops_try_microservice_kit(bite_scaffold bite_scaffold::bite_scaffold _found)
    if(_found)
        return()
    endif()

    add_library(skillops::microservice_kit INTERFACE IMPORTED GLOBAL)
    set(SKILLOPS_MICROSERVICE_KIT_PACKAGE "none" CACHE STRING "Resolved C++ microservice kit package")
    set(SKILLOPS_MICROSERVICE_KIT_TARGET "skillops::microservice_kit" CACHE STRING "Resolved C++ microservice kit target")
    message(WARNING
        "Unable to find C++ microservice kit. Building the first-stage skeleton "
        "with the standard-library fallback. Install cpp-microservice-kit to enable "
        "framework logging and adapters.")
endfunction()

skillops_find_microservice_kit()
