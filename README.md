# PC-Studio-3-SMS-Deserialize
This application aims to extract SMS messages and their associated metadata from the binary database Samsung PC Studio 3 uses.


###Where is this database?
The database is in your Application Data directory. In Windows Explorer, type `%AppData%` in the address bar. Once in that folder, go to the Samsung folder, then the PC Studio folder. There should be a file called `SMS.msg`. That's where everything is.


###Can Outlook open this file?
No.


###What format is this file in?
It's not a standard format as far as I know. I did my best to detail the format of the file [here](https://docs.google.com/document/u/1/d/1JSrFQPBYiSIkekzTrc1Gw5CH57R9UovVlj9rNToaZ2Y/pub). It may not be correct and there may be some parts missing (the "unknown" parts may actually mean something), but it works enough to pull the important data out of it.


###What is this compatible with?
I think it's compatabible with anything that can compile C code with the standard C libraries. That being said, I haven't tried this on Windows. The development computer was a 64-bit Ubuntu system.


###How do I build this?
Clone the repository and then run the following command:

`gcc SMSDeserialize.c -o SMSDeserialize`

It's that simple. If you're using another compiler, then it should work just as simply as well. It uses no weird standards (not even C99), so it should compile with almost anything.


###How do I use it?
`SMSDeserialize <infile> <outfile>`

where `infile` is the file you copied from the directory above, and outfile is the name of the file you want to write to.


###What does this output?
The program outputs CSV without headers. The format is as follows:

`message number,direction(in or out),unix timestamp,phone number,"message body"`

If you want a human readable time, you can change that in the code by removing comments


###Why does this application print "Unknown" for the time sometimes?
You'll notice it only does that when there is no time specified. PC Studio doesn't save the outgoing time (pity), so I just set it to say "Unknown" because it's lost with time.
