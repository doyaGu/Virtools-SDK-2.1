/*************************************************************************/
/*	File : CKSprite.h													 */
/*	Author :  Romain Sididris											 */
/*																		 */
/*	Virtools SDK 														 */
/*	Copyright (c) Virtools 2000, All Rights Reserved.					 */
/*************************************************************************/
#if !defined(CKSPRITE_H) || defined(CK_3DIMPLEMENTATION)
#define CKSPRITE_H "$Id:$"
#ifndef CK_3DIMPLEMENTATION

#include "CK2dEntity.h"
#include "CKBitmapData.h"

#undef CK_PURE
#define CK_PURE = 0

/*************************************************
{filename:CKSprite}
Name: CKSprite

Summary: Sprite Class.
Remarks:
    + A sprite is special kind of 2D entity that supports displaying
    non power of 2 images (limitation of textures)

    + This class provides the basic methods for loading a sprite from a file, access
    the surface data,specify transparent color.

    + The class id of CKSprite is CKCID_SPRITE.



See also: CK2dEntity,CKSpriteText,CKBitmapData
*************************************************/
class CKSprite : public CK2dEntity
{
public:
#endif

    /*************************************************
    Summary: Creates an empty image in the sprite.

    Arguments:
        Width: width in pixel of the image to create
        Height: height in pixel of the image to create
        BPP: Bit per pixel of the image
        Slot: If there a multiple images, index of the image slot to create.
    Return Value:
        TRUE if successful, FALSE otherwise.
    Remarks:
    + The name for the created slot is set to ""
    + If there was already some slots created but with a different width or height
    the method returns FALSE.
    See Also:LoadImage,SaveImage
    *************************************************/
    virtual CKBOOL Create(int Width, int Height, int BPP = 32, int Slot = 0) CK_PURE;

    /*************************************************
    Summary: Loads a image slot from a file.

    Arguments:
        Name: Name of the file to load.
        Slot: In a multi-images sprite, index of the image slot to load.
    Return Value:
        TRUE if successful, FALSE otherwise.
    Remarks:
        + The supported image formats depend on the image readers installed. The default available
        readers support BMP,TGA,JPG,PNG and GIF format
    See also: SaveImage
    *************************************************/
    virtual CKBOOL LoadImage(CKSTRING Name, int Slot = 0) CK_PURE;

    /************************************************
    Summary: Saves an image slot to a file.

    Arguments:
        Name: Name of the file to save.
        Slot: In a multi-images sprite,index of the image slot to save.
        UseFormat: If set force usage of the save format specified in SetSaveFormat, otherwise use extension given in Name. Default : FALSE
    Return Value: TRUE if successful, FALSE otherwise.
    Remarks:
        + The image format depends on the image readers installed. The default available
        readers support BMP,TGA,JPG,PNG and GIF format

    See also: CreateImage,CKBitmapReader
    ************************************************/
    virtual CKBOOL SaveImage(CKSTRING Name, int Slot = 0, BOOL CKUseFormat = FALSE) CK_PURE;

    //-------------------------------------------------------------
    // Movie Loading

    /************************************************
    Summary: Creates a multi-image sprite from a movie file.

    Arguments:
        Name: Movie file to load.
        width: width of the sprite to create.
        height:	height of the sprite to create.
        Bpp: Number of bit per pixel of the sprite to create.
    Return Value: TRUE if loading was successful, FALSE otherwise.
    Remarks:
        + The supported movie formats depend on the image readers and codecs installed. The default available
        reader support AVI files.
    See also: LoadImage,Create
    ************************************************/
    virtual CKBOOL LoadMovie(CKSTRING Name, int width = 0, int height = 0, int Bpp = 16) CK_PURE;

    /*************************************************
    Summary: Returns the name of the movie file used by this sprite.

    Return Value:
        A pointer to the name of the file which was used to load this sprite or NULL if this sprite is not a movie.
    See also: LoadMovie,GetMovieReader
    *************************************************/
    virtual CKSTRING GetMovieFileName() CK_PURE;

    /*************************************************
    Summary: Returns the movie reader used to decompress the current movie.
    Return value: Pointer to CKMovieReader
    Remarks:
        + This method returns a movie reader if one is present,NULL otherwise.

    See also: LoadMovie,GetMovieFileName,CKMovieReader
    *************************************************/
    virtual CKMovieReader *GetMovieReader() CK_PURE;

    //-------------------------------------------------------------
    // SURFACE PTR ACCES
    // Acces to Surface Ptr ,
    // Once Locked a surface ptr must be release
    // for modification on the texture to be available
    // if the operation is read-only no release is required

    /*************************************************
    Summary: Returns a pointer to the image surface buffer.
    Arguments:
        Slot: In a multi-images texture, index of the image slot to get surface pointer of. -1 means the current active slot.
    Return Value:
         A valid pointer to the texture buffer or NULL if failed.
    Remarks:
    + The return value is a pointer on the system memory copy of the sprite.
    + If any changes are made (write access) to the image surface, you must either call Restore which immediatly copies the sprite back in video memory or ReleaseSurfacePtr() which flags this sprite as to be reloaded before it is used next time.

    See also: ReleaseSurfacePtr,SetPixel,GetPixel
    *************************************************/
    virtual CKBYTE *LockSurfacePtr(int Slot = -1) CK_PURE;

    /*************************************************
    Summary: Marks a slot as modified.
    Return Value:
        TRUE if successful.
    Arguments:
        Slot: In a multi-images sprite, number of the image slot to mark as invalid.
    Remarks:
    + When changes are made to the bitmap data (using LockSurfacePtr or SetPixel) this method marks the changed slot so that they can reloaded in video memory when necessary.

    See also: LockSurfacePtr,SetPixel
    *************************************************/
    virtual CKBOOL ReleaseSurfacePtr(int Slot = -1) CK_PURE;
    //-------------------------------------------------------------
    // Bitmap filenames information

    /*************************************************
    Summary: Returns the name of the file used to load an image slot.
    Arguments:
        slot: image slot index
    Return value: A pointer to the name of the file which was used to load this image slot

    See also: SetSlotFileName
    *************************************************/
    virtual CKSTRING GetSlotFileName(int Slot) CK_PURE;

    /*************************************************
    Summary: Sets the name of the file used to load an image slot.
    Arguments:
        Slot: image slot index
        Filename: image slot file name
    Return Value: TRUE if successful, FALSE otherwise.

    See also: GetSlotFileName
    *************************************************/
    virtual CKBOOL SetSlotFileName(int Slot, CKSTRING Filename) CK_PURE;

    //--------------------------------------------------------------
    // Bitmap storage information

    /*************************************************
    Summary: Gets the image width
    Return value: Image width

    See Also: GetBytesPerLine,GetHeight,GetBitsPerPixel
    *************************************************/
    virtual int GetWidth() CK_PURE;

    /*************************************************
    Summary: Gets the image height
    Return Value: Image height

    See Also: GetBytesPerLine,GetWidth,GetBitsPerPixel
    *************************************************/
    virtual int GetHeight() CK_PURE;

    virtual int GetBitsPerPixel() CK_PURE;

    virtual int GetBytesPerLine() CK_PURE;

    virtual int GetRedMask() CK_PURE;

    virtual int GetGreenMask() CK_PURE;

    virtual int GetBlueMask() CK_PURE;

    virtual int GetAlphaMask() CK_PURE;

    //-------------------------------------------------------------
    // Image slot count

    /************************************************
    Summary: Returns the number of slot (images) in this sprite.
    Return Value: Number of images.
    Remarks:

    See also: SetSlotCount,GetCurrentSlot,SetCurrentSlot
    ************************************************/
    virtual int GetSlotCount() CK_PURE;

    /************************************************
    Summary: Sets the number of slot (images) in this sprite.
    Arguments:
        Count: Image slots  to allocate.
    Return Value: TRUE if successful, FALSE otherwise.
    Remarks:

    See also: GetSlotCount,GetCurrentSlot,SetCurrentSlot
    ************************************************/
    virtual CKBOOL SetSlotCount(int Count) CK_PURE;

    /************************************************
    Summary: Sets the current active image.

    Arguments:
        Slot: Image slot index.
    Return Value:
        TRUE if successful, FALSE otherwise.
    See also: GetSlotCount,SetSlotCount,GetCurrentSlot
    *****************************************************/
    virtual CKBOOL SetCurrentSlot(int Slot) CK_PURE;

    /************************************************
    Summary: Returns current slot index.

    Return Value: Current image slot index.
    See Also:GetSlotCount,SetSlotCount,SetCurrentSlot
    ************************************************/
    virtual int GetCurrentSlot() CK_PURE;

    /************************************************
    Summary: Removes an image.

    Return Value:
        TRUE if successful, FALSE otherwise.
    Arguments:
        Slot: Index of the image to remove.
    See also: GetSlotCount,GetCurrentSlot,SetCurrentSlot
    ************************************************/
    virtual CKBOOL ReleaseSlot(int Slot) CK_PURE;

    /************************************************
    Summary: Deletes all the images.

    Return Value:
        TRUE if successful, FALSE otherwise.
    See also: GetSlotCount,GetCurrentSlot,SetCurrentSlot
    ************************************************/
    virtual CKBOOL ReleaseAllSlots() CK_PURE;

    //-------------------------------------------------------------
    // ACCES TO SYSTEM MEMORY SURFACE

    /*************************************************
    Summary: Sets the color of a pixel.
    Return Value:
        TRUE if successful, FALSE otherwise.
    Arguments:
        x: X position of the pixel to set the color of.
        y: Y position of the pixel to set the color of.
        col: A Dword ARGB color to set
        slot: Index of the slot in which the pixel should be set or -1 for the current slot.
    Remarks:
    + There is no check on the validity of x or y parameter so its the user responsability.
    + Sets the color of a pixel in the copy of the texture in system memory.
    + Changes will only be visible after using Restore() function to force the sprite to re-load from system memory.

    See Also:LockSurfacePtr,GetPixel,ReleaseSurfacePtr
    *************************************************/
    virtual CKBOOL SetPixel(int x, int y, CKDWORD col, int slot = -1) CK_PURE;

    /*************************************************
    Summary: Gets the color of a pixel.
    Arguments:
        x: X position of the pixel to set the color of.
        y: Y position of the pixel to set the color of.
        slot: Index of the slot in which the pixel should be get or -1 for the current slot.
    Return Value: Color of the pixel (32 bit ARGB)
    Remarks:
        + There is no check on the validity of x or y parameter so its the
        user responsability.

    See Also:LockSurfacePtr,SetPixel
    *************************************************/
    virtual CKDWORD GetPixel(int x, int y, int slot = -1) CK_PURE;

    //-------------------------------------------------------------
    // TRANSPARENCY

    /************************************************
    Summary: Returns the transparent color.
    Return Value:
        Color: A 32 bit ARGB transparent color.
    Remarks:
        + 0x00000000 (black) is the default transparent color.

    See also: SetTranparentColor,SetTransparent
    ************************************************/
    virtual CKDWORD GetTransparentColor() CK_PURE;

    /************************************************
    Summary: Sets the transparent color.
    Arguments:
        Color: A 32 bit ARGB transparent color.
    Remarks:
    + 0x00000000 (black) is the default transparent color.
    + Setting on the transparency and a transparent color automatically
    updates the alpha channel so that pixel with the transparent color have
    a 0 alpha value.

    See also: GetTranparentColor,SetTransparent
    ************************************************/
    virtual void SetTransparentColor(CKDWORD Color) CK_PURE;

    /************************************************
    Summary: Enables or disables the color key transparency.
    Arguments:
        Transparency: TRUE activates transparency, FALSE disables it.
    Remarks:
    + 0x00000000 (black) is the default transparent color.
    + Setting on the transparency and a transparent color automatically
    updates the alpha channel so that pixel with the transparent color have
    a 0 alpha value.

    See also: IsTransparent,SetTranparentColor
    ************************************************/
    virtual void SetTransparent(CKBOOL Transparency) CK_PURE;

    /************************************************
    Summary: Returns whether color keyed transparency is enabled.
    Return Value:
        TRUE if transparency is enabled.
    Arguments:
        Transparency: TRUE activates transparency, FALSE disables it.
    Remarks:

    See also: IsTransparent
    ************************************************/
    virtual CKBOOL IsTransparent() CK_PURE;

    //-------------------------------------------------------------
    // VIDEO MEMORY MANAGEMENT

    /*************************************************
    Summary: Restore sprite video memory

    Return Value:
        TRUE if successful, FALSE otherwise.
    Remarks:
        + If the sprite is already in video memory, this function copies
        the content of the system memory data to video memory otherwise it fails.
    See also: SystemToVideoMemory
    ************************************************/
    virtual CKBOOL Restore(CKBOOL Clamp = FALSE) CK_PURE;

    /*************************************************
    Summary: Allocates and copies the sprite image from system to video memory.

    Arguments:
        Dev: A pointer to a CKRenderContext on which the sprite is to be used.
        Clamping: used internally.
    Return Value:
        TRUE if successful, FALSE otherwise.
    Remarks:
    + This method is automatically called by the framework when a sprite
    needs to be drawn and is not present in video memory.
    + Use this function to creates the video memory copy.
    + Usually, FreeVideoMemory is called for sprites that won't be visible for a long time. When one of
    these sprites should be used again, calling SystemToVideoMemory ensures that the sprite
    will be stored into video memory.
    See also: FreeVideoMemory,Restore
    ************************************************/
    virtual CKBOOL SystemToVideoMemory(CKRenderContext *Dev, CKBOOL Clamping = FALSE) CK_PURE;

    /*************************************************
    Summary: Frees the sprite video memory.

    Return Value:
        TRUE if successful, FALSE otherwise.
    Remarks:
    + Use this function to manage which sprites should be stored in video memory. Some video
    cards may render sprites stored in sytem memory (ie. AGP).
    + This method can be used in low video memory configuration
    to flush sprites that have not been used for a long time.
    See also: SystemToVideoMemory
    ************************************************/
    virtual CKBOOL FreeVideoMemory() CK_PURE;

    /*************************************************
    Summary: Returns whether the sprite is present in video memory

    Return Value:
        TRUE if the sprite is in video memory FALSE otherwise.
    See also: SystemToVideoMemory
    *************************************************/
    virtual CKBOOL IsInVideoMemory() CK_PURE;

    virtual CKBOOL CopyContext(CKRenderContext *ctx, VxRect *Src, VxRect *Dest) CK_PURE;

    //-------------------------------------------------------------

    /*************************************************
    Summary: Returns a information about how sprite is stored in video memory.

    Arguments:
        desc: A reference to a VxImageDescEx structure that will be filled with video memory format information.
    Return Value:
        FALSE if the sprite is not in video memory or invalid, TRUE otherwise.
    See Also: GetVideoPixelFormat,SetDesiredVideoFormat
    *************************************************/
    virtual CKBOOL GetVideoTextureDesc(VxImageDescEx &desc) CK_PURE;

    /*************************************************
    Summary: Returns the pixel format of the sprite surface in video memory.

    Return Value:
        Pixel format of video memory surface (VX_PIXELFORMAT) or VX_UNKNOWNPF
        if the sprite is not in video memory.
    See Also: SetDesiredVideoFormat, GetVideoTextureDesc
    *************************************************/
    virtual VX_PIXELFORMAT GetVideoPixelFormat() CK_PURE;

    /*************************************************
    Summary: Gets Image format description

    Output Arguments:
        desc: A reference to a VxImageDescEx structure that will be filled with system memory format information.
    Return Value:
        TRUE.
    Remarks:
    + The desc parameter will be filled with the size,pitch,bpp and mask information on how the sprite is stored in system memory.
    See Also: GetVideoPixelFormat,SetDesiredVideoFormat
    *************************************************/
    virtual CKBOOL GetSystemTextureDesc(VxImageDescEx &desc) CK_PURE;

    //-------- Expected Video Format ( textures will use global texture format ottherwise )

    /*************************************************
    Summary: Sets the system memory caching option.
    Arguments:
        iOptions: System Caching Options.
    Remarks:

    See Also: SetSaveFormat,CK_BITMAP_SYSTEMCACHING
    *************************************************/
    virtual void SetSystemCaching(CK_BITMAP_SYSTEMCACHING iOptions) CK_PURE;

    virtual CK_BITMAP_SYSTEMCACHING GetSystemCaching() CK_PURE;

    //-------- Save format

    virtual CK_BITMAP_SAVEOPTIONS GetSaveOptions() CK_PURE;

    /*************************************************
    Summary: Sets the saving options.

    Input Arguments:
        Options: Save Options.
    Remarks:
    + When saving a composition sprites can kept as reference to external files or
    converted to a given format and saved inside the composition file. The CK_BITMAP_SAVEOPTIONS
    enumeration exposes the available options.
    See Also: SetSaveFormat,CK_BITMAP_SAVEOPTIONS
    *************************************************/
    virtual void SetSaveOptions(CK_BITMAP_SAVEOPTIONS Options) CK_PURE;

    virtual CKBitmapProperties *GetSaveFormat() CK_PURE;
    /*************************************************
    Summary: Sets the saving format.
    Arguments:
        format: A CKBitmapProperties that contain the format in which the bitmap should be saved.
    Remarks:
    + If the save options have been set to CKTEXTURE_IMAGEFORMAT you can specify a
    format in which the sprite will be converted before being saved inside the composition file.
    + The CKBitmapProperties structure contains the CKGUID of a BitmapReader that is to be
    used plus some additionnal settings specific to each format.

    See Also: SetSaveOptions,CKBitmapProperties,CKBitmapReader
    *************************************************/
    virtual void SetSaveFormat(CKBitmapProperties *format) CK_PURE;

    /*************************************************
    Summary: Sets pick threshold value.
    Arguments:
        pt: Pick threshold value to be set.
    Return Value:
    Remarks:
    + The pick threshold is used when picking transparent sprites.
    + It is the minimum value for alpha component
    below which picking is not valid.So this value is supposed to be in the range 0..255
    and the default value 0 means the picking is always valid.
    + But if a value >0 is used and the texture use transparency (some pixels of the bitmap will have
    alpha component of 0) a sprite will not be picked on its transparent part.

    See Also: CKRenderContext::Pick
    *************************************************/
    virtual void SetPickThreshold(int pt) CK_PURE;

    virtual int GetPickThreshold() CK_PURE;

    virtual CKBOOL ToRestore() CK_PURE;

    /*************************************************
    Summary: Dynamic cast operator.
    Arguments:
        iO: A pointer to a CKObject to cast.
    Return Value:
        iO casted to the appropriate class or NULL if iO is not from the required class .
    Example:
          CKObject* Object;
          CKSprite* anim = CKSprite::Cast(Object);
    Remarks:

    *************************************************/
    static CKSprite *Cast(CKObject *iO)
    {
        return CKIsChildClassOf(iO, CKCID_SPRITE) ? (CKSprite *)iO : NULL;
    }

#ifndef CK_3DIMPLEMENTATION
};

#endif
#endif
