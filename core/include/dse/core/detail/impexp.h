#ifndef DSE_CORE_DETAIL_IMPEXP_H
#define DSE_CORE_DETAIL_IMPEXP_H

#ifdef _MSC_VER
#define API_EXPORT_DSE __declspec(dllexport)
#define API_IMPORT_DSE __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#define API_EXPORT_DSE
#define API_IMPORT_DSE
#endif

#ifdef DSE_CORE_EXPORT
#define API_DSE_CORE API_EXPORT_DSE
#else
#define API_DSE_CORE API_IMPORT_DSE
#endif

#endif // DSE_CORE_DETAIL_IMPEXP_H
