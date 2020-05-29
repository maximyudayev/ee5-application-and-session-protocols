TITLE_COLOR = \033[33m
NO_COLOR = \033[0m
FLAGS = -Wall -std=c11 -Werror -fdiagnostics-color=auto
DEBUG_LVL = 1
INPUTS = 100 100 1

# when executing make, compile all exe's
all: clean-all coap_client

# When trying to compile one of the executables, first look for its .c files
# Then check if the libraries are in the lib folder
coap_client: main.c
	@echo "$(TITLE_COLOR)\n***** CPPCHECK *****$(NO_COLOR)"
	cppcheck --enable=all --suppress=missingIncludeSystem main.c
	@echo "$(TITLE_COLOR)\n***** COMPILING COAP CLIENT *****$(NO_COLOR)"
	gcc main.c -D DEBUG_LVL -o coap_client $(FLAGS) -lpthread

# do not look for files called clean, clean-all or this will be always a target
.PHONY: clean clean-all 

clean:
	rm -rf sensor_log* *.png *.html ./coverage/*

clean-all: clean
	rm -rf *.o lib/*.o lib/*.so coap_client *~ 

leak: all
	@echo "$(TITLE_COLOR)\n***** LEAK CHECK COAP CLIENT *****$(NO_COLOR)"
	valgrind --leak-check=full -v --track-origins=yes --show-leak-kinds=all ./coap_client $(INPUTS)

run:
	@echo "$(TITLE_COLOR)\n***** RUNNING COAP CLIENT *****$(NO_COLOR)"
	./coap_client $(INPUTS)

clean-coverage:
	@echo -e '\n*********************************'
	@echo -e '*** Cleanup of coverage files ***'
	@echo -e '*********************************'
	rm -f ./*.gcda ./*.gcno

coverage: clean-all clean-coverage
	./coap_client $(INPUTS)
	lcov --capture --directory . --output-file ./coverage/coverage.info
	genhtml ./coverage/coverage.info --output-directory ./coverage
	make clean-coverage

show-coverage: coverage
	firefox ./coverage/index.html