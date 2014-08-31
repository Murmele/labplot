#ifndef PARSER_EXTERN_H
#define PARSER_EXTERN_H

extern "C" double parse(const char[]);
extern "C" int parse_errors();
extern "C" void init_table();
extern "C" void delete_table();
extern "C" void* assign_variable(const char* variable, double value);

extern "C" struct con _constants[];
extern "C" struct func _functions[];

#endif /* PARSER_EXTERN_H */
