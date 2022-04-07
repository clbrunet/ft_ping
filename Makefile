NAME := ft_ping
SRCS_DIR := ./src
SRCS_EXTENSION := c
BUILD_DIR := ./build

CC := clang
CFLAGS := -Wall -Wextra -Werror
CFLAGS += -I./src/
CFLAGS += -g3 -fsanitize=address
# CFLAGS += -DNDEBUG

SRCS := $(shell find $(SRCS_DIR) -type f -name "*.$(SRCS_EXTENSION)")
OBJS := $(SRCS:$(SRCS_DIR)/%.$(SRCS_EXTENSION)=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all
all: $(NAME)

-include $(DEPS)

$(BUILD_DIR)/%.o: $(SRCS_DIR)/%.$(SRCS_EXTENSION) Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -o $@ -c $<

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(NAME) $(OBJS) $(LDLIBS)

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

.PHONY: fclean
fclean: clean
	$(RM) $(NAME)

.PHONY: re
re: fclean all
