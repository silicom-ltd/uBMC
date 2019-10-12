
#include <signal.h>
#include <setjmp.h>
#include "silc_common.h"
#include "editline/readline.h"
#include "silc_cli_common.h"
#include "silc_cli_terminal.h"
#include "silc_cli_error.h"


static char s_silc_cli_rl_name[32];
static silc_list s_silc_cli_rl_avail_token_list;
static silc_list *s_p_silc_cli_rl_avail_token_list;
static silc_cli_token* s_p_silc_cli_rl_token_curr;
static silc_bool s_silc_cli_rl_help_enable = silc_false;
static silc_bool s_silc_cli_rl_recv_sigint = silc_false;

void silc_cli_rl_single_quotes_space_convert(silc_cstr cstr)
{
	int i;
	int len = strlen(cstr);
	silc_bool quote_start = silc_false;

	for(i=0; i<len; i++)
	{
		if(i+1 >= len)
		{
			if(quote_start && cstr[i] == ' ')
				cstr[i] = 28;
			return;
		}

		if(quote_start)
		{
			if(cstr[i] == '\'' && cstr[i+1] == ' ')
				quote_start = silc_false;
			else if(cstr[i] == ' ')
				cstr[i] = 28;
		}
		else
		{
			if(cstr[i] == ' ' && cstr[i+1] == '\'')
				quote_start = silc_true;
		}
	}
}

void silc_cli_rl_single_quotes_space_revert(silc_cstr cstr)
{
	int i;
	int len = strlen(cstr);

	for(i=0; i<len; i++)
	{
		if(cstr[i] == 28)
			cstr[i] = ' ';
	}
}

/*
 * Tell the GNU Readline library how to complete.  We want to try to
 * complete on command names if this is the first word in the line, or
 * on filenames if not.
 */
int silc_cli_rl_help_enable(int count, int c)
{
	s_silc_cli_rl_help_enable = silc_true;
	rl_newline(0, 0);
	return 0;
}


void silc_cli_rl_signal_handler(int sig_num)
{
	silc_cli_print("^C");
	s_silc_cli_rl_recv_sigint = silc_true;
}

void silc_cli_rl_add_history(silc_cstr line);
char **silc_cli_rl_completion(const char* text, int start, int end);
void silc_cli_rl_init(const silc_cstr name)
{
	rl_initialize();

	/*
	 * Allow conditional parsing of the ~/.inputrc file.
	 */
	strncpy(s_silc_cli_rl_name, name, 32);
	rl_readline_name = s_silc_cli_rl_name;

	/*
	 * Tell the completer that we want a crack first.
	 */
	rl_attempted_completion_function = silc_cli_rl_completion;
	//rl_keyboard_input_hook = cli_clear_auto_logout;

	rl_bind_key('?', silc_cli_rl_help_enable);
	//int l_max, c_max;
	//rl_get_screen_size(&l_max, &c_max);
	//printf("[%s]terminal name:%s\n", __func__, rl_get_terminal_name());
	signal(SIGINT, silc_cli_rl_signal_handler);
}

void silc_cli_rl_stripwhite(silc_cstr str);
void silc_cli_rl_echo_readline(silc_cstr line);
silc_cstr silc_cli_rl_cut_cmd_prefix(silc_cstr line);
void silc_cli_rl_show_help(silc_cstr cmd_prefix, const silc_cstr text);
void silc_rl_keyboard_input_hook(void)
{
	silc_cli_session_timeout_update();
}

silc_cstr silc_cli_rl_readline()
{
	static char last_cmd[1000]="";
	silc_cstr line, text;
	rl_keyboard_input_hook = silc_rl_keyboard_input_hook;
	while((line = readline(silc_cli_mode_get_curr_prompt())))
	{
		if(s_silc_cli_rl_recv_sigint)
		{
			s_silc_cli_rl_recv_sigint = silc_false;
			continue;
		}
		if(!line)
			continue;

		silc_cli_terminal_clear();
		silc_cli_rl_stripwhite(line);

		if(s_silc_cli_rl_help_enable)	// print help
		{
			silc_cli_rl_echo_readline(line);
			silc_cli_rl_single_quotes_space_convert(line);
			text = silc_cli_rl_cut_cmd_prefix(line);
			if(text == line)
				silc_cli_rl_show_help("", text);
			else
				silc_cli_rl_show_help(line, text);
			s_silc_cli_rl_help_enable = silc_false;
			free(line);
			continue;
		}

		if(line[0] == 0)	// input null
		{
			free(line);
			continue;
		}
		else	// input command
    	   break;
	}
	if(line)
	{
		if(strncmp(last_cmd, line, 1000) != 0)
		{
			silc_cli_rl_add_history(line);
			strcpy(last_cmd, line);
		}
	}
	return line;
}

silc_cstr silc_cli_rl_dupstr(silc_cstr s)
{
	silc_cstr r;

	r = malloc(strlen(s) + 1);
	strcpy(r, s);
	return r;
}

silc_bool silc_cli_rl_is_undisplay(silc_cli_token* p_token)
{
#if 0
	if(p_token->p_parent->type != TOKEN_TYPE_MODE)	// only undisplay first level command
		return silc_false;
	if(strcmp(p_token->name, "debug") == 0 ||
			strcmp(p_token->name, "manufacture") == 0)
		return silc_true;
	return silc_false;
#else
	return ((p_token->hidden & 1<<SILC_MGMTD_PRODUCT_ID) != 0);
#endif
}

char* silc_cli_rl_token_gen(const char* text, int state)
{
	static int len;
	static silc_cli_token* p_token;
	static silc_list_node* p_node;
	silc_bool is_optional = silc_false;
#ifdef ENABLE_PRINT_CR
	static silc_bool is_finished = silc_false;	// print list with <cr> for finished command
#endif

	if(s_p_silc_cli_rl_avail_token_list == &s_silc_cli_rl_avail_token_list)
	{
		is_optional = silc_true;
	}

  	if(!state)
    {
  		len = strlen (text);
  		p_node = s_p_silc_cli_rl_avail_token_list->next;
#ifdef ENABLE_PRINT_CR
  		if(is_optional)
  			return silc_cli_rl_dupstr("<cr>");
  		if(p_node == s_p_silc_cli_rl_avail_token_list)
  		{
  			is_finished = silc_true;
  			return silc_cli_rl_dupstr("<cr>");
  		}
#endif
    }

  	while(p_node != s_p_silc_cli_rl_avail_token_list)
    {
  		if(is_optional)
  			p_token = silc_list_entry(p_node, silc_cli_token, rl_node);
  		else
  			p_token = silc_list_entry(p_node, silc_cli_token, node);
  		p_node = p_node->next;
  		if(strncmp(p_token->name, text, len) == 0 && !silc_cli_rl_is_undisplay(p_token))
  			return silc_cli_rl_dupstr(p_token->name);
    }

    /*
     * If no names matched, then return NULL.
     */
#ifdef ENABLE_PRINT_CR
  	if(is_finished)
  	{
		is_finished = silc_false;
		return silc_cli_rl_dupstr("");
  	}
#endif
    return NULL;
}

char* silc_cli_rl_token_value_gen(const char* text, int state)
{
	static int len;
	static int index;
	silc_cstr name = NULL;
	silc_cli_token* p_cur_token = s_p_silc_cli_rl_token_curr;

	/*
	 * If this is a new word to complete, initialize now.  This
     * includes saving the length of TEXT for efficiency, and
     * initializing the index variable to 0.
     */
  	if(!state)
    {
  		len = strlen (text);
  		index = 0;
  		name = p_cur_token->dync_val_str_buf;
    }
  	else
  	{
  		index++;
  		name = p_cur_token->dync_val_str_buf + index*p_cur_token->dync_val_str_len;
  	}

  	/*
  	 * Return the next name which partially matches from the
     * command list.
     */
  	while(index < p_cur_token->dync_val_str_num)
    {
  		name = p_cur_token->dync_val_str_buf + index*p_cur_token->dync_val_str_len;
  		if(strncmp(name, text, len) == 0)
  			return silc_cli_rl_dupstr(name);
  		index++;
    }

    /*
     * If no names matched, then return NULL.
     */
    return NULL;
}



void silc_cli_rl_stripwhite(silc_cstr str)
{
	int i;
	silc_bool need_skip = silc_true;
	int len = strlen(str);
	silc_cstr cur = str;

	//cli_mprintf("[%s] original:%s\n", __func__, str);
	for(i=0; i<len; i++)
	{
		if(need_skip && str[i] == ' ')
			continue;
		if(str[i] == ' ')
			need_skip = silc_true;
		else
			need_skip = silc_false;
		*cur = str[i];
		cur++;
	}
	*cur = 0;
	//cli_mprintf("[%s] current:%s\n", __func__, str);
}

void silc_cli_rl_print_type_str_help(silc_cstr type_str, silc_bool insert_space)
{
	if(insert_space)
		silc_cli_print("Type \"%s ?\" for help.\n", type_str);
	else
		silc_cli_print("Type \"%s?\" for help.\n", type_str);
}


silc_cstr silc_cli_rl_get_readline_valid_prefix(silc_cstr_array* cmd_array, int length)
{
	int loop;
	static char buffer[1024];

	strcpy(buffer, "");
	for(loop=0; loop<length; loop++)
	{
		sprintf(buffer+strlen(buffer), "%s ", silc_cstr_array_get_quick(cmd_array, loop));
	}
	return buffer;
}

void silc_cli_rl_free_token_list(silc_list* p_token_list)
{
	silc_cli_token *p_token, *p_tmp_token;

	silc_list_for_each_entry_safe(p_token, p_tmp_token, p_token_list, rl_node)
	{
		if(p_token->type != TOKEN_TYPE_STATIC && p_token->val_str)
		{
			free(p_token->val_str);
			p_token->val_str = NULL;
		}
		silc_list_del(&p_token->rl_node);
	}
}

#define MAX_PATH_LENGTH	1024
silc_cli_cmd* silc_cli_rl_parser_cmd(silc_cstr cmd_str, silc_list* p_token_list)
{
	char path[MAX_PATH_LENGTH];
	int loop;
	silc_cstr item;
	silc_cstr_array* p_arr;
	silc_cli_token* p_token;
	silc_cli_cmd* p_cmd = NULL;

	silc_cli_rl_single_quotes_space_convert(cmd_str);
	p_arr = silc_cstr_split(cmd_str, " ");
	if(!p_arr)
		return NULL;

	silc_list_init(p_token_list);
	strcpy(path, silc_cli_mode_curr_name_get());
	// check tokens
	for(loop=0; loop<p_arr->length; loop++)
	{
		item = silc_cstr_array_get_quick(p_arr, loop);
		if(strlen(path)+1+strlen(item) >= MAX_PATH_LENGTH)
		{
			silc_cli_print("%% Too long command \"%s %s\", type \"?\" for help.\n", path, item);
			goto ERROR;
		}
		strcat(path, " ");
		strcat(path, item);
		p_token = silc_cli_token_find(path);
		if(!p_token)
		{
			if(loop)
				silc_cli_print("%% Unknown parameter \"%s\", type \"%s?\" for help.\n",
					item, silc_cli_rl_get_readline_valid_prefix(p_arr, loop));
			else
				silc_cli_print("%% Unknown command \"%s\", type \"?\" for help.\n", item);
			goto ERROR;
		}

		if(p_token->cmd_func)
			p_cmd = p_token;

		if(!p_cmd)
		{
			silc_cli_print("%% Unknown command \"%s\", type \"?\" for help.\n",
					silc_cli_rl_get_readline_valid_prefix(p_arr, loop+1));
			goto ERROR;
		}

		if(p_token->type == TOKEN_TYPE_VARIABLE || p_token->type == TOKEN_TYPE_DYNAMIC)
		{
			if((loop + 1) == p_arr->length)	// unfinished
			{
				silc_cli_print("%% Incomplete command, type \"%s?\" for help.\n",
						silc_cli_rl_get_readline_valid_prefix(p_arr, loop+1));
				goto ERROR;
			}
			else // duplicate value string
			{
				item = silc_cstr_array_get_quick(p_arr, loop+1);
				p_token->val_str = malloc(strlen(item) + 1);
				if(!p_token->val_str)
				{
					silc_cli_print("%% Memory is not enough, Unable allocate memory for parameter \"%s\", Check you system.\n", item);
					goto ERROR;
				}
				silc_cli_rl_single_quotes_space_revert(item);
				strcpy(p_token->val_str, item);
				loop++;	// skip token value
			}
		}
		silc_list_add_tail(&p_token->rl_node, p_token_list);
		//printf("list:%p, p_token:%p, node:%p\n", p_token_list, p_token, &p_token->rl_node);
		if(p_token->type == TOKEN_TYPE_COMMAND)
		{
			char args[MAX_PATH_LENGTH] = {0};
			int i;
			for(i=loop+1; i<p_arr->length; i++)
			{
				item = silc_cstr_array_get_quick(p_arr, i);
				if(strlen(args)+1+strlen(item) >= MAX_PATH_LENGTH)
				{
					silc_cli_print("%% Too long args \"%s %s\", type \"?\" for help.\n", args, item);
					goto ERROR;
				}
				strcat(args, item);
				strcat(args, " ");
			}
			p_token->val_str = malloc(strlen(args) + 1);
			if(!p_token->val_str)
			{
				silc_cli_print("%% Memory is not enough, Unable allocate memory for parameter \"%s\", Check you system.\n", args);
				goto ERROR;
			}
			strcpy(p_token->val_str, args);
			break;
		}
	}
	if(p_token)
	{
		if(p_token->sub_mode != TOKEN_MODE_OPTIONAL &&
				!silc_list_empty(&p_token->sub_token_list))
		{
			silc_cli_print("%% Incomplete command, type \"%s?\" for help.\n",
					silc_cli_rl_get_readline_valid_prefix(p_arr, loop));
			goto ERROR;
		}
	}
	silc_cstr_array_destroy(p_arr);
	return p_cmd;

ERROR:
	silc_cstr_array_destroy(p_arr);
	silc_cli_rl_free_token_list(p_token_list);
	return NULL;
}

int silc_cli_rl_parser_cmd_prefix(const silc_cstr prefix, silc_cli_token** pp_cur_token, silc_bool* p_next_is_value)
{
	char path[255], opt_path[255];
	int cur_item_num = 0;
	silc_cstr item;
	silc_cstr_array* p_arr;
	silc_cli_token* p_token = NULL;
	silc_bool find_optional = silc_false;

	s_p_silc_cli_rl_avail_token_list = NULL;
	silc_list_init(&s_silc_cli_rl_avail_token_list);

	*pp_cur_token = NULL;
	*p_next_is_value = silc_false;

	if(strlen(prefix) == 0)
	{
		*pp_cur_token = silc_cli_mode_curr_get();
		s_p_silc_cli_rl_avail_token_list = &(*pp_cur_token)->sub_token_list;
		return 0;
	}

	p_arr = silc_cstr_split(prefix, " ");
	strcpy(path, silc_cli_mode_curr_name_get());

	// check tokens
	while(cur_item_num < p_arr->length)
	{
		item = silc_cstr_array_get_quick(p_arr, cur_item_num++);
		if(!find_optional)
		{
			strcat(path, " ");
			strcat(path, item);
		}
		else
		{
			sprintf(path, "%s %s", opt_path, item);
		}
		p_token = silc_cli_token_find(path);
		if(!p_token)
		{
//			silc_cli_rl_print_type_str_help(silc_cli_rl_get_readline_valid_prefix(p_arr, cur_item_num-1), silc_true);
			goto ERROR;
		}

		if(find_optional)
		{
			silc_cli_token* p_sub_token;
			silc_cli_token* p_tmp_token;
			silc_list_for_each_entry_safe(p_sub_token, p_tmp_token, s_p_silc_cli_rl_avail_token_list, rl_node)
			{
				if(strcmp(p_sub_token->name, p_token->name) == 0)
					silc_list_del(&p_sub_token->rl_node);
			}
		}

		if(p_token->sub_mode == TOKEN_MODE_OPTIONAL)
		{
			silc_cli_token* p_sub_token;
			find_optional = silc_true;
			s_p_silc_cli_rl_avail_token_list = &s_silc_cli_rl_avail_token_list;
			strcpy(opt_path, path);
			silc_list_for_each_entry(p_sub_token, &p_token->sub_token_list, node)
			{
				silc_list_add_tail(&p_sub_token->rl_node, s_p_silc_cli_rl_avail_token_list);
			}
		}

		if(p_token->type == TOKEN_TYPE_VARIABLE || p_token->type == TOKEN_TYPE_DYNAMIC) // skip the value string
		{
			if(cur_item_num == p_arr->length)	// finished check token
			{
				*p_next_is_value = silc_true;
				break;
			}
			else
				cur_item_num++;	// skip token value
		}
	}

	*pp_cur_token = p_token;
//	if(!find_optional)
		s_p_silc_cli_rl_avail_token_list = &p_token->sub_token_list;
	silc_cstr_array_destroy(p_arr);
	return 0;

ERROR:
	silc_cstr_array_destroy(p_arr);
	return -1;
}

void silc_cli_rl_get_token_dync_value_string(const silc_cstr prefix)
{
	silc_cli_token* p_cur_token = s_p_silc_cli_rl_token_curr;
	void* func_data = NULL;
	char data[100];

	if(!p_cur_token)
		return;

	if(p_cur_token->type == TOKEN_TYPE_DYNAMIC)
	{
		if(p_cur_token->dync_func)
		{
			func_data = p_cur_token->dync_func_data;
			if(p_cur_token->dync_func_data_idx)
			{
				silc_cstr_array* p_arr = silc_cstr_split(prefix, " ");
				if(!p_arr)
					return;
				if(p_arr->length >= p_cur_token->dync_func_data_idx)
				{
					strcpy(data, silc_cstr_array_get_quick(p_arr, p_cur_token->dync_func_data_idx));
					func_data = data;
				}
				silc_cstr_array_destroy(p_arr);
			}
			p_cur_token->dync_val_str_num = p_cur_token->dync_func(func_data,
					p_cur_token->dync_val_str_buf, &p_cur_token->dync_val_str_len);
		}
	}
	else
		p_cur_token->dync_val_str_num = 0;
}

char **silc_cli_rl_token_completion(silc_cstr prefix, const silc_cstr text)
{
	silc_bool is_value;
	char **matches = NULL;

	if(silc_cli_rl_parser_cmd_prefix(prefix, &s_p_silc_cli_rl_token_curr, &is_value) < 0)
		return NULL;

	if(is_value)	// text is token value
	{
		if(s_p_silc_cli_rl_token_curr->type == TOKEN_TYPE_DYNAMIC)
		{
			silc_cli_rl_get_token_dync_value_string(prefix);
			matches = rl_completion_matches(text, silc_cli_rl_token_value_gen);
		}
	}
	else	// text is token
	{
		matches = rl_completion_matches(text, silc_cli_rl_token_gen);
	}

	return matches;
}


/*
 * Attempt to complete on the contents of TEXT.  START and END
 * bound the region of rl_line_buffer that contains the word to
 * complete.  TEXT is the word to complete.  We can use the entire
 * contents of rl_line_buffer in case we want to do some simple
 * parsing.  Returnthe array of matches, or NULL if there aren't any.
 */
char **silc_cli_rl_completion(const char* text, int start, int end)
{
#if 0
	char **matches = NULL;
	char prefix[strlen(rl_line_buffer)+1];

	// get prefix from readline buffer
	strcpy(prefix, rl_line_buffer);
	prefix[start] = 0;
	silc_cli_rl_stripwhite(prefix);
	printf("\n%s, %s\n", prefix, text);
	matches = silc_cli_rl_token_completion(prefix, text);

	return matches;
#else
	char **matches = NULL;
	char buff[strlen(rl_line_buffer)+1];
	char prefix[strlen(rl_line_buffer)+1];
	silc_cstr last;

//	if(text)
//		printf("[%s] text:%s, start:%d, end:%d\n", __func__, text, start, end);

	// get prefix from readline buffer
	strcpy(buff, rl_line_buffer);
	buff[start] = 0;
	sprintf(buff+strlen(buff), "%s", text);
	silc_cli_rl_single_quotes_space_convert(buff);
//	printf("\n%s\n", buff);
	last = silc_cli_rl_cut_cmd_prefix(buff);
	if(last == buff)
		prefix[0] = 0;
	else
		sprintf(prefix, "%s ", buff);
	silc_cli_rl_stripwhite(prefix);
//	printf("\n%s, %s\n", prefix, last);
	matches = silc_cli_rl_token_completion(prefix, last);

	return matches;
#endif
}

void silc_cli_rl_print_token_help(silc_cli_token *p_cur_token, silc_list* p_avail_token_list, const silc_cstr token_name)
{
	int len = strlen(token_name);
	silc_cli_token *p_token;

	if(silc_list_empty(p_avail_token_list))
	{
		silc_cli_print("%-32s%s\n", "<cr>", p_cur_token->comment);
		return;
	}

	if(s_p_silc_cli_rl_avail_token_list == &s_silc_cli_rl_avail_token_list)
	{
		silc_cli_print("%-32s%s\n", "<cr>", p_cur_token->comment);
		silc_list_for_each_entry(p_token, p_avail_token_list, rl_node)
		{
			if(strncmp(token_name, p_token->name, len) == 0 && !silc_cli_rl_is_undisplay(p_token))
				silc_cli_print("%-32s%s\n", p_token->name, p_token->comment);
		}
	}
	else
	{
		silc_list_for_each_entry(p_token, p_avail_token_list, node)
		{
			if(strncmp(token_name, p_token->name, len) == 0 && !silc_cli_rl_is_undisplay(p_token))
				silc_cli_print("%-32s%s\n", p_token->name, p_token->comment);
		}
	}
}

void silc_cli_rl_print_token_value_help(silc_cli_token* p_token, const silc_cstr prefix, const silc_cstr text)
{
	int loop;
	int len = strlen(text);
	silc_cstr val_str;
	silc_bool find_val = silc_false;

	if(p_token->type == TOKEN_TYPE_DYNAMIC)
	{
		silc_cli_rl_get_token_dync_value_string(prefix);
		if(p_token->dync_val_str_num == 0)
		{
			//silc_cli_print("%s\n", p_token->val_help);
			silc_cli_print("The list is empty\n");
			return;
		}

		for(loop=0; loop<p_token->dync_val_str_num; loop++)
		{
			val_str = p_token->dync_val_str_buf + p_token->dync_val_str_len*loop;
			if(strncmp(text, val_str, len) == 0)
			{
				silc_cli_print("%s\n", val_str);
				find_val = silc_true;
			}
		}
		if(!find_val)
			silc_cli_print("%-32s%s\n", text, "Unknown value");
	}
	else if(p_token->type == TOKEN_TYPE_VARIABLE)
	{
		silc_cli_print("%-32s%s\n", "<variable>", p_token->val_help);
	}
}


void silc_cli_rl_show_help(silc_cstr prefix, const silc_cstr text)
{
	silc_bool is_value;

	if(silc_cli_rl_parser_cmd_prefix(prefix, &s_p_silc_cli_rl_token_curr, &is_value) < 0)
		return;

	if(is_value)	// text is token value
	{
		silc_cli_rl_print_token_value_help(s_p_silc_cli_rl_token_curr, prefix, text);
	}
	else	// text is token
	{
		silc_cli_rl_print_token_help(s_p_silc_cli_rl_token_curr, s_p_silc_cli_rl_avail_token_list, text);
	}
}

void silc_cli_rl_echo_readline(silc_cstr line)
{
	int loop;
	int len = strlen(line);

   	for(loop=0; loop<len; loop++)
   	{
   		rl_insert(1, line[loop]);
   	}
}

silc_cstr silc_cli_rl_cut_cmd_prefix(silc_cstr line)
{
	int loop;

	for(loop=strlen(line); loop>0; loop--)
	{
		if(line[loop] == ' ')
		{
			line[loop] = 0;
			return &line[loop+1];
		}
	}
	return line;
}

void silc_cli_rl_add_history(silc_cstr line)
{
	add_history(line);
}

void silc_cli_rl_clear_history()
{
	clear_history();
}

void silc_cli_rl_free_readline(silc_cstr line)
{
	free(line);
}


