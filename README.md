dgnlib-1.1
==========

DGNLib: a Microstation DGN (ISFF) Reader

I (Frank Warmerdam) have implemented a Microstation DGN reading library on contract to Avenza Systems Inc., makers of MAPublisher. The library will be OpenSource (under my usual X/MIT license), portable and callable from C/C++. While it has been incorporated into my OGR Simple Features library, it is also be usable standalone with minimal overhead. The dgnlib library, and this page of DGN related information are primarily hosted at http://dgnlib.maptools.org.
DGNLib

DGNLib is a small C/C++ library for reading and writing DGN files.
Where can I get the source code?
dgnlib-1.11.zip: Current standalone source with dgndump example mainline.

Does DGNLib support all DGN elements?
No, but it does support most 2D and 3D elements. Some of the more esoteric elements are read in "raw data mode", but not interpreted. It does support lines, line strings, curves, bsplines, ellipses, arcs, and text elements, as well as extracting color tables, and master coordinate information.

Does DGNLib support Microstation V8 DGN Files?
No, they are a substantially different format, and are not recognised at all. The DGNdirect library from the OpenDWG Alliance does support DGN v8.

Is it portable?
Yes, fairly, though as distributed it may be necessary to tweak the cpl_port.h and cpl_config.h files for different platforms. It was developed on Linux for a client deploying on MacOS, and Windows.

Does it require alot of memory?
Generally speaking no. Only one element at a time is kept in memory. If the file is "indexed" an additional 12 bytes per element is kept in memory after a pre-scan, but this isn't required.

Does DGNLib include code to draw DGN elements?
No, but there is a document that tries to indicate how to properly draw DGN elements given the structures returned by DGNLib. The OGR layer re-interprets the drawing information into OGR Feature Style format which is understood by some applications such as UMN MapServer.

What is the license?
DGNLib is under my usual MIT/X style open source license. Thus is can be easily used in commercial, and free products with no concern about licensing issues. See the header of any source file for the full license text.

Is there support for writing DGN files?
Yes, there is preliminary support for writing 2D and 3D DGN files in recent releases.

Is there documentation?
There is a detailed DGN Library API Reference available. There is currently no tutorial style information on how to use the library, but the dgndump.c utility can serve as a limited example.

Is further work going on to improve the library?
The initial project is complete, but I am interested in fixing bugs and adding minor features. Improvements are welcome.

DGN Related Information

ISFF/DGN File Format: I have taken the venerable isff.txt description, and am reformatting it in HTML and filling in missing information.
Microstation 95/ISFF Description (pdf): Information on the ISFF/DGN file format as distributed with Microstation 95.
The most recently committed code is available in GDAL CVS tree.
DGN Library API Documentation
DGN Feature Representation: info on drawing DGN elements properly.
Intergraph Notes on Database Linkages (also db_link.doc in MS Word format)
The dgnlib@yahoogroups.com mailing list is for discussion of general DGN issues as well as dgnlib in particular. The group includes an email archive and can be subscribed from the following subscribe box. Please confirm your interest with me by email if you are using a relatively anonymous email service so I can verify you aren't a spammer.
Subscribe to dgnlib
	Click here to join dgnlib
Other DGN Viewers/Libraries

DGNdirect is a library from the Open Design Alliance that supports reading (and to some extent) writing DGN v8 files. It also supports reading DGN v7 files. The Open Design Alliance offers it under quite reasonable (but not Open Source) licensing terms.
Bentley itself offers DGN libraries, and tools as part of their OpenDGN program.
GraphStore has a product called eCADLite that can be used in demo mode as a DGN viewer.
Columbus is a free document viewer which includes DGN support.
AutoVue from Cimmetry.
Safe Software's FME product has excellent DGN read and write support, and can be used as a standalone product, or FMEObjects acts as a library.
My own OpenEV uses dgnlib for DGN support.
Sample Datasets

dgnsamp.zip: DGN Samples from Cybertal
Ohio DOT Downloads Page
Oregon DOT
Credits

I would like to thank:
Dennis Christopher (and Avenza Systems Inc) for funding this development, and allowing me to release it as open source.
Rodney Jenson who gave me some hints, and moral support.
Patrick van Dijk, for information and code related to Quaternion handling.
Cybertal Components, whose's OpenDGN based dumping program was helpful to me in testing. Unfortunately, Cybertal does not appear to still be in business.
Ason Kang who provided a sample multi-byte text file and code to handle it properly.
The various US DOTs and other government organizations who are progressive enough to provide mapping information directly in DGN format on the web. It's a great store of sample data.
Mike Cane for held with Radix50 names, and support for cell headers.
Jeff Smith (Geographic Software Specialists) who funded DGN write support.
Jamie Cecchetto (Century Systems) for funding 3D write support.
DM Solutions Group for providing the maptools.org site to host this on, and for funding (also thanks to Tyler Mitchell of Lignum) integration into OGR.
Marius Kintel (Systems In Motion) for adding support for reading 3D cone (23), 3D surface (18) and 3D solid (19) elements.
