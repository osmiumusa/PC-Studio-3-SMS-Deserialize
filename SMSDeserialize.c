/**
Samsung PC Studio 3 SMS Database Parser
Written by Tucker Osman, 2016
This application uses the Apache Liscense 2.0
**/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

void read_doublewide(char*,FILE*,unsigned char);
void read_doublewide_long(char*,FILE*,unsigned short);
void OLE_to_datestring(double,char*);
char* escapeCSV(char*);

int main(int argc, char** argv) {
	if(argc!=3) {
		printf("Usage: %s <input> <output>\n\n",argv[0]);
		exit(1);
	}

	FILE* infile; //File to read from
	FILE* outfile; //File to write to
	unsigned short messages; //How many messages we have
	unsigned short messages_left; //So we don't have to deal with the footer
	unsigned char message_type; //1=incoming, 2=outgoing
	unsigned char message_length; //Length of message
	unsigned short message_llength; //If the message is unconformantly long.
	char* message_body = malloc(1024); //If you have a SMS longer than this, change and recompile.
	unsigned char phonenumber_length; //Length of phone number.
	char* phonenumber = malloc(20); //Shouldn't be longer than this
	double oledate; //OLE Date in raw form is a double
	unsigned short message_number; //ID of the number or something like that
	unsigned char folder_length; //Length of folder name
	char* folder_name = malloc(128); //I don't care about this. Do you?
	unsigned char tainted = 0; //Solve an inconsistency
	char* datestring = malloc(48); //Date format shouldn't be longer than this
	char* csvmsg; //We'll escape the string

	//Open the files
	infile = fopen(argv[1],"r");
	outfile = fopen(argv[2],"w");
	if(infile==NULL||outfile==NULL) {
		printf("There was a problem with file I/O. :(\n\n");
		exit(2);
	}

	//Read the input file
	//First we need to read the header.
	fseek(infile,5,SEEK_SET); //Skip the 5 byte pad
	fread(&messages,sizeof(short),1,infile); //Message Count
	messages_left = messages;
	printf("Processing %d messages...\n",messages);

	fseek(infile,16,SEEK_CUR); //Skip over the object name in an unintellegent way.
	fread(&message_type,sizeof(char),1,infile); //Message Type

	fseek(infile,6,SEEK_CUR); //Skip 3 byte pad and field boundary
	fread(&message_length,sizeof(char),1,infile); //Message Length
	read_doublewide(message_body,infile,message_length); //Read the message correctly

	fseek(infile,7,SEEK_CUR); //Skip 4 byte pad and field boundary
	fread(&phonenumber_length,sizeof(char),1,infile); //Length of phone number
	read_doublewide(phonenumber,infile,phonenumber_length); //Read the phone number correctly

	fseek(infile,10,SEEK_CUR); //Skip 2 unknown, field boundary, and 5 pad bytes
	fread(&oledate,sizeof(double),1,infile); //Read the OLE Date of message

	fseek(infile,18,SEEK_CUR); //Skip over the object name in an unintellegent way.
	fread(&message_number,sizeof(short),1,infile); //Message id

	fseek(infile,21,SEEK_CUR); //Skip over the object name and field boundary in an unintellegent way.
	fread(&folder_length,sizeof(char),1,infile); //Length of the folder name
	read_doublewide(folder_name,infile,folder_length); //Yeah but who cares

	fseek(infile,15,SEEK_CUR); //Skip right to the end of the record.

	//Write everything to file
	OLE_to_datestring(oledate,datestring);
	csvmsg = escapeCSV(message_body);
	fprintf(outfile,"%d,%s,%s,%s,%s\n",message_number,(message_type==1)?"in":"out",datestring,phonenumber,csvmsg);
	free(csvmsg);
	messages_left--;

	//Now let's loop through the standard structures!
	for(;messages_left>0;messages_left--) {
		printf("\r%d messages left...  ",messages_left);
		fseek(infile,14,SEEK_CUR); //Skip over the pad and boundary
		fread(&message_type,sizeof(char),1,infile); //Message Type

		fseek(infile,6,SEEK_CUR); //Skip 3 byte pad and field boundary
		fread(&message_length,sizeof(char),1,infile); //Message Length

		if(message_length==255) { //If the number here is 255, then a short follows with the real length
			fread(&message_llength,sizeof(short),1,infile);
			read_doublewide_long(message_body,infile,message_llength);
		} else {
			read_doublewide(message_body,infile,message_length); //Read the message correctly
		}

		fseek(infile,7,SEEK_CUR); //Skip 4 byte pad and field boundary
		fread(&phonenumber_length,sizeof(char),1,infile); //Length of phone number

		if(phonenumber_length==0) { //Weird inconsistency
			fseek(infile,5,SEEK_CUR);
			fread(&phonenumber_length,sizeof(char),1,infile);
			tainted = 1;
		}

		read_doublewide(phonenumber,infile,phonenumber_length); //Read the phone number correctly

		fseek(infile,10,SEEK_CUR); //Skip 2 unknown, field boundary, and 5 pad bytes
		fread(&oledate,sizeof(char),8,infile); //Read the OLE Date of message

		if(tainted==0) fseek(infile,6,SEEK_CUR); //Skip over the unknown bytes

		fread(&message_number,sizeof(short),1,infile); //Message id
		fseek(infile,11,SEEK_CUR); //Skip over the unknown bytes and field boundary

		fread(&folder_length,sizeof(char),1,infile); //Length of the folder name
		read_doublewide(folder_name,infile,folder_length); //Yeah but who cares

		fseek(infile,6,SEEK_CUR); //Skip right to the end of the record.

		//Write everything to file
		OLE_to_datestring(oledate,datestring);
		csvmsg = escapeCSV(message_body);
		fprintf(outfile,"%d,%s,%s,%s,%s\n",message_number,(message_type==1)?"in":"out",datestring,phonenumber,csvmsg);
		free(csvmsg);
		tainted = 0;
	}
	fflush(outfile); //Make sure everything is written.
	free(message_body); //Free what we've allocated
	free(phonenumber);
	free(folder_name);
	free(datestring);
	printf("\nDone!\n");
}

//This format stores characters as shorts, so skip every other zero.
void read_doublewide(char* dest, FILE* infile, unsigned char length) {
	int i;
	for(i=0;i<length;i++) {
		fread(dest+i,sizeof(char),1,infile); //Read a character
		fseek(infile,1,SEEK_CUR); //Dump the 0
	}
	dest[length] = 0;
}

void read_doublewide_long(char* dest, FILE* infile, unsigned short length) {
	int i;
	for(i=0;i<length;i++) {
		fread(dest+i,sizeof(char),1,infile); //Read a character
		fseek(infile,1,SEEK_CUR); //Dump the 0
	}
	dest[length] = 0;
}

//Convert an OLE Date double to a formated string.
void OLE_to_datestring(double ole, char* out) {
	if((int)ole==0) { //Sent messages have no timestamp :(
		strcpy(out,"Unknown");
		return;
	}
	time_t seconds = (time_t)(-2209161600 + (long int)(ole*24*60*60));
	asctime_r(gmtime(&seconds),out);
	out[strlen(out)-1] = 0;
}

//http://stackoverflow.com/a/22284011/1461223
char* escapeCSV(char* in) {
	int in_len = strlen(in);
	char *out_buf = malloc(in_len*2+3);
	int out_idx = 0;
	int in_idx = 0;

	out_buf[out_idx++] = '"';
	for(in_idx=0; in_idx < in_len; in_idx++) {
		if(in[in_idx] == '"') {
			out_buf[out_idx++] = '"';
			out_buf[out_idx++] = '"';
		} else {
			out_buf[out_idx++] = in[in_idx];
		}
	}

	out_buf[out_idx++] = '"';
	out_buf[out_idx++] = 0;
	return out_buf;
}
