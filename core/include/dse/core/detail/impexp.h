#ifndef DSE_CORE_DETAIL_IMPEXP_H
#define DSE_CORE_DETAIL_IMPEXP_H

#define API_EXPORT_DSE
#define API_IMPORT_DSE

#ifdef DSE_CORE_EXPORT
#define API_DSE_CORE API_EXPORT_DSE
#else
#define API_DSE_CORE API_IMPORT_DSE
#endif

#endif // DSE_CORE_DETAIL_IMPEXP_H
