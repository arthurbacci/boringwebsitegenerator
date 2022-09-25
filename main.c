#include <stdio.h>
#include <stdbool.h>

bool is_whitespace(char c) {
	if (c == ' ' || c == '\t')
		return true;
	return false;
}

void print_escaped(char c, FILE *wp) {
	switch (c) {
	case '&':
		fputs("&amp", wp);
		break;
	case '<':
		fputs("&lt", wp);
		break;
	case '>':
		fputs("&gt", wp);
		break;
	case '"':
		fputs("&quot", wp);
		break;
	case '\'':
		fputs("&#39", wp);
		break;
	default:
		fputc(c, wp);
		break;
	}
}

void passline(FILE *fp, FILE *wp) {
	int c;
	while ((c = fgetc(fp)) != EOF && c != '\n')
		print_escaped(c, wp);
}

int main(int argc, char **argv) {
	FILE *fp, *wp;
	bool pre_on = false;
	int c;
	
	if (argc < 2)
		return 1;
		
	fp = fopen(argv[1], "r");
	wp = stdout;
	while ((c = fgetc(fp)) != EOF) {

		if (c == '`') {
			int c1, c2;
			c1 = fgetc(fp);
			c2 = fgetc(fp);
			if (c1 == '`' && c2 == '`') {
				if (!pre_on) {
					fputs("<pre title=\"", wp);
					passline(fp, wp);
					fputs("\">", wp);
					passline(fp, wp);
				} else {
					fputs("</pre>", wp);
				}
				pre_on = !pre_on;
				continue;
			} else {
				fseek(fp, -2, SEEK_CUR);
			}
		}
		if (pre_on) {
			fseek(fp, -1, SEEK_CUR);
			print_escaped('\n', wp);
			passline(fp, wp);
			continue;
		}
		
		switch (c) {
		case '\n':
			fputs("<br>", wp);
			break;
		case '>':
			fputs("<blockquote>", wp);
			
			passline(fp, wp);
			
			fputs("</blockquote>", wp);
			break;
		case '*':
			c = fgetc(fp); /* mandatory space */
			
			fputs("<span>❧ ", wp);
			
			passline(fp, wp);
			
			fputs("</span><br>", wp);
			break;
		case '#': {
			int heading = 1;
			while ((c = fgetc(fp)) == '#')
				heading++;
			
			fprintf(wp, "<h%d>", heading);
			
			passline(fp, wp);
			
			fprintf(wp, "</h%d>", heading);
			break;
		}
		case '=': {
			int c1 = fgetc(fp);
			if (c1 == '>') {
				fpos_t pos;
			
				while (is_whitespace(c = fgetc(fp)));
				fseek(fp, -1, SEEK_CUR);
				
				fgetpos(fp, &pos);
				
				fputs("<a href=\"", wp);
				while (!is_whitespace(c = fgetc(fp)) && c != EOF)
					print_escaped(c, wp);
				fputs("\">", wp);
				
				fsetpos(fp, &pos);
				
				passline(fp, wp);
				
				fputs("</a>", wp);
			
				break;
			} else {
				fseek(fp, -2, SEEK_CUR);
			}
		}
		default:
			fputs("<p>", wp);
			fseek(fp, -1, SEEK_CUR);
			passline(fp, wp);
			fputs("</p>", wp);
			break;
		}
	}
	
	fclose(fp);

	return 0;
}
