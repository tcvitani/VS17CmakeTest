1.0.0 24.10.2003 TCV

   - Module creation

1.0.1 19.01.2004 TCV

   - Storing of decoded images has been changed. The images are stored in the sub-directories
     depending on their date and time of acquisition. The procedure for the image purging has 
     been added.

1.0.2 02.09.2005 TCV

   - The function DateToDays has been corrected.
   - More file format checking has been added. If the picture size is zero the module doesn't
     try to extract the picture from GEA file but it removes the GEA file and reports an 
     error. 