OBJECTS = main.o ftl_config_manager.o ftl_mapping_manager.o ftl.o ftl_gc_manager.o ftl_buffer.o ftl_inverse_mapping_manager.o ssd_io_manager.o ssd_log_manager.o ftl_cache.o

test : $(OBJECTS)
	gcc -o test $(OBJECTS)

main.o : main.c
	gcc -c main.c

ftl.o : ftl.h ftl.c 
	gcc -c ftl.c

ftl_config_manager.o : ftl_config_manager.c ftl_config_manager.h
	gcc -c ftl_config_manager.c

ftl_mapping_manager.o : ftl_mapping_manager.c ftl_mapping_manager.h
	gcc -c ftl_mapping_manager.c

ftl_gc_manager.o : ftl_gc_manager.c ftl_gc_manager.h
	gcc -c ftl_gc_manager.c

ftl_buffer.o : ftl_buffer.c ftl_buffer.h
	gcc -c ftl_buffer.c

ftl_cache.o : ftl_cache.c ftl_cache.h
	gcc -c ftl_cache.c

ftl_inverse_mapping_manager.o : ftl_inverse_mapping_manager.c ftl_inverse_mapping_manager.h
	gcc -c ftl_inverse_mapping_manager.c

ssd_io_manager.o : ssd_io_manager.c ssd_io_manager.h
	gcc -c ssd_io_manager.c

ssd_log_manager.o : ssd_log_manager.c ssd_log_manager.h
	gcc -c ssd_log_manager.c



clean:
	rm *.o
