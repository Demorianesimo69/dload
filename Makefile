

# //////////////////////////////////////////////////////////////////////////////
#
# By the Project Diamond authors.
# DLOAD Makefile
#
# Revision 0.02: Now with EFI. :)
#
# //////////////////////////////////////////////////////////////////////////////

.PHONY: all mbr cleanmbr efi cleanefi

all: mbr efi

clean: cleanmbr cleanefi

mbr:
	echo "Entering MBR Source directory" && cd mbr/ && \
	./build && echo "MBR compile complete!" && \
	echo "Exiting MBR Source directory" && cd ..

cleanmbr:
	echo "Entering MBR Source directory" && cd mbr/ && \
	rm -rfv dload-* && echo "MBR Source clean complete!" && \
	echo "Exiting MBR Source directory" && cd ..

efi:
	echo "Entering EFI Source directory" && cd efi/ && \
	./build && echo "EFI compile complete!" && \
	echo "Exiting EFI Source directory" && cd ..

cleanefi:
	echo "Entering EFI Source directory" && cd efi/ && \
	rm -rfv dload-* && echo "EFI Source clean complete!" && \
	echo "Exiting EFI Source directory" && cd ..

