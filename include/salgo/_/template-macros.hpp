#pragma once


#define SALGO_REQUIRES(...)    class = std::enable_if_t<(__VA_ARGS__)>

#define SALGO_REQUIRES_CV_OF(A,B) SALGO_REQUIRES( std::is_same_v< std::remove_cv_t< A >, B > )
