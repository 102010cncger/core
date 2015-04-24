#ifndef _EMF_FILE_H
#define _EMF_FILE_H

#include "../Wmf/WmfUtils.h"
#include "../Wmf/WmfTypes.h"

#include "../Common.h"

#include "EmfTypes.h"
#include "EmfOutputDevice.h"
#include "EmfPlayer.h"
#include "EmfPath.h"

#include "../../../fontengine/FontManager.h"
#include <iostream>

namespace MetaFile
{
	class CEmfFile
	{
	public:

		CEmfFile() : m_oPlayer(this)
		{
			m_pBufferData = NULL;
			m_bError      = false;
			m_pOutput     = NULL;
			m_pPath       = NULL;

			m_oStream.SetStream(NULL, 0);

			m_pDC = m_oPlayer.GetDC();
		};

		~CEmfFile()
		{
			Close();
		};

		bool OpenFromFile(const wchar_t* wsFilePath)
		{
			Close();

			NSFile::CFileBinary oFile;
			oFile.OpenFile(wsFilePath);
			int lFileSize = oFile.GetFileSize();

			m_pBufferData = new BYTE[lFileSize];
			if (!m_pBufferData)
				return false;

			DWORD lReadedSize;
			oFile.ReadFile(m_pBufferData, lFileSize, lReadedSize);

			m_oStream.SetStream(m_pBufferData, lFileSize);

			return true;
		}
		void Close()
		{
			RELEASEOBJECT(m_pBufferData);
			RELEASEOBJECT(m_pPath);

			m_pOutput = NULL;
			m_oStream.SetStream(NULL, 0);
			m_bError = false;
			m_oPlayer.Clear();
			m_pDC = m_oPlayer.GetDC();
		}
		TEmfRectL* GetBounds()
		{
			return &m_oHeader.oFrame;
		}
		void SetOutputDevice(CEmfOutputDevice* pOutput)
		{
			m_pOutput = pOutput;
		}
		void Scan()
		{
			CEmfOutputDevice* pOutput = m_pOutput;
			m_pOutput = NULL;
			PlayMetaFile();
			m_pOutput = pOutput;

			RELEASEOBJECT(m_pPath);
			m_oPlayer.Clear();
			m_pDC = m_oPlayer.GetDC();
		}
		bool CheckError()
		{
			return m_bError;
		}
		void SetFontManager(CFontManager* pManager)
		{
			m_pFontManager = pManager;
		}
		void PlayMetaFile()
		{
			if (!m_oStream.IsValid())
				SetError();

			m_lTest = 0;
			unsigned int ulSize, ulType;
			unsigned int ulNumber = 0;

			bool bEof = false;

			unsigned int ulRecordIndex = 0;

			if (m_pOutput)
				m_pOutput->Begin();

			do
			{
				if (m_oStream.CanRead() < 8)
					return SetError();

				m_oStream >> ulType;
				m_oStream >> ulSize;

				m_ulRecordSize = ulSize - 8;

				if (ulType < EMR_MIN || ulType > EMR_MAX)
					return SetError();

				if (0 == ulRecordIndex && EMR_HEADER != ulType)
					return SetError();

				switch (ulType)
				{
					//-----------------------------------------------------------
					// 2.3.1 Bitmap
					//-----------------------------------------------------------
					case EMR_BITBLT:            Read_EMR_BITBLT(); break;
					case EMR_STRETCHDIBITS:     Read_EMR_STRETCHDIBITS(); break;
					case EMR_SETDIBITSTODEVICE: Read_EMR_SETDIBITSTODEVICE(); break;
						//-----------------------------------------------------------
						// 2.3.2 Clipping
						//-----------------------------------------------------------
					case EMR_EXTSELECTCLIPRGN:  Read_EMR_EXTSELECTCLIPRGN(); break;
					case EMR_INTERSECTCLIPRECT: Read_EMR_INTERSECTCLIPRECT(); break;
					case EMR_SELECTCLIPPATH:    Read_EMR_SELECTCLIPPATH(); break;
					case EMR_SETMETARGN:        Read_EMR_SETMETARGN(); break;
						//-----------------------------------------------------------
						// 2.3.4 Control
						//-----------------------------------------------------------
					case EMR_HEADER: Read_EMR_HEADER(); break;
					case EMR_EOF:    Read_EMR_EOF(); bEof = true; break;
						//-----------------------------------------------------------
						// 2.3.5 Drawing
						//-----------------------------------------------------------
					case EMR_ANGLEARC:          Read_EMR_ANGLEARC(); break;
					case EMR_ARC:               Read_EMR_ARC(); break;
					case EMR_ARCTO:             Read_EMR_ARCTO(); break;
					case EMR_CHORD:             Read_EMR_CHORD(); break;
					case EMR_ELLIPSE:           Read_EMR_ELLIPSE(); break;
					case EMR_EXTTEXTOUTA:       Read_EMR_EXTTEXTOUTA(); break;
					case EMR_EXTTEXTOUTW:       Read_EMR_EXTTEXTOUTW(); break;
					case EMR_FILLPATH:          Read_EMR_FILLPATH(); break;
					case EMR_LINETO:            Read_EMR_LINETO(); break;
					case EMR_PIE:               Read_EMR_PIE(); break;
					case EMR_POLYBEZIER:        Read_EMR_POLYBEZIER(); break;
					case EMR_POLYBEZIER16:      Read_EMR_POLYBEZIER16(); break;
					case EMR_POLYBEZIERTO:      Read_EMR_POLYBEZIERTO(); break;
					case EMR_POLYBEZIERTO16:    Read_EMR_POLYBEZIERTO16(); break;
					case EMR_POLYDRAW:          Read_EMR_POLYDRAW(); break;
					case EMR_POLYDRAW16:        Read_EMR_POLYDRAW16(); break;
					case EMR_POLYGON:           Read_EMR_POLYGON(); break;
					case EMR_POLYGON16:         Read_EMR_POLYGON16(); break;
					case EMR_POLYLINE:          Read_EMR_POLYLINE(); break;
					case EMR_POLYLINE16:        Read_EMR_POLYLINE16(); break;
					case EMR_POLYLINETO:        Read_EMR_POLYLINETO(); break;
					case EMR_POLYLINETO16:      Read_EMR_POLYLINETO16(); break;
					case EMR_POLYPOLYGON:       Read_EMR_POLYPOLYGON(); break;
					case EMR_POLYPOLYGON16:     Read_EMR_POLYPOLYGON16(); break;
					case EMR_POLYPOLYLINE:      Read_EMR_POLYPOLYLINE(); break;
					case EMR_POLYPOLYLINE16:    Read_EMR_POLYPOLYLINE16(); break;
					case EMR_POLYTEXTOUTA:      Read_EMR_POLYTEXTOUTA(); break;
					case EMR_POLYTEXTOUTW:      Read_EMR_POLYTEXTOUTW(); break;
					case EMR_RECTANGLE:         Read_EMR_RECTANGLE(); break;
					case EMR_ROUNDRECT:         Read_EMR_ROUNDRECT(); break;
					case EMR_SETPIXELV:         Read_EMR_SETPIXELV(); break;
					case EMR_SMALLTEXTOUT:      Read_EMR_SMALLTEXTOUT(); break;
					case EMR_STROKEANDFILLPATH: Read_EMR_STROKEANDFILLPATH(); break;
					case EMR_STROKEPATH:        Read_EMR_STROKEPATH(); break;
						//-----------------------------------------------------------
						// 2.3.7 Object Creation
						//-----------------------------------------------------------
					case EMR_CREATEBRUSHINDIRECT:     Read_EMR_CREATEBRUSHINDIRECT(); break;
					case EMR_CREATEDIBPATTERNBRUSHPT: Read_EMR_CREATEDIBPATTERNBRUSHPT(); break;
					case EMR_CREATEPALETTE:           Read_EMR_CREATEPALETTE(); break;
					case EMR_CREATEPEN:               Read_EMR_CREATEPEN(); break;
					case EMR_EXTCREATEFONTINDIRECTW:  Read_EMR_EXTCREATEFONTINDIRECTW(); break;
					case EMR_EXTCREATEPEN:            Read_EMR_EXTCREATEPEN(); break;
						//-----------------------------------------------------------
						// 2.3.8 Object Manipulation
						//-----------------------------------------------------------
					case EMR_SELECTOBJECT:  Read_EMR_SELECTOBJECT(); break;
					case EMR_DELETEOBJECT:  Read_EMR_DELETEOBJECT(); break;
					case EMR_SELECTPALETTE: Read_EMR_SELECTPALETTE(); break;
						//-----------------------------------------------------------
						// 2.3.10 Path Bracket
						//-----------------------------------------------------------
					case EMR_BEGINPATH:   Read_EMR_BEGINPATH(); break;
					case EMR_ENDPATH:     Read_EMR_ENDPATH(); break;
					case EMR_CLOSEFIGURE: Read_EMR_CLOSEFIGURE(); break;
					case EMR_FLATTENPATH: Read_EMR_FLATTENPATH(); break;
					case EMR_WIDENPATH:   Read_EMR_WIDENPATH(); break;
					case EMR_ABORTPATH:   Read_EMR_ABORTPATH(); break;
						//-----------------------------------------------------------
						// 2.3.11 State
						//-----------------------------------------------------------
					case EMR_MOVETOEX:          Read_EMR_MOVETOEX(); break;
					case EMR_SAVEDC:            Read_EMR_SAVEDC(); break;
					case EMR_RESTOREDC:         Read_EMR_RESTOREDC(); break;
					case EMR_SETTEXTCOLOR:      Read_EMR_SETTEXTCOLOR(); break;
					case EMR_SETTEXTALIGN:      Read_EMR_SETTEXTALIGN(); break;
					case EMR_SETBKMODE:         Read_EMR_SETBKMODE(); break;
					case EMR_SETMITERLIMIT:     Read_EMR_SETMITERLIMIT(); break;
					case EMR_SETPOLYFILLMODE:   Read_EMR_SETPOLYFILLMODE(); break;
					case EMR_SETMAPMODE:        Read_EMR_SETMAPMODE(); break;
					case EMR_SETWINDOWORGEX:    Read_EMR_SETWINDOWORGEX(); break;
					case EMR_SETWINDOWEXTEX:    Read_EMR_SETWINDOWEXTEX(); break;
					case EMR_SETVIEWPORTORGEX:  Read_EMR_SETVIEWPORTORGEX(); break;
					case EMR_SETVIEWPORTEXTEX:  Read_EMR_SETVIEWPORTEXTEX(); break;
					case EMR_SETBKCOLOR:        Read_EMR_SETBKCOLOR(); break;
					case EMR_SETSTRETCHBLTMODE: Read_EMR_SETSTRETCHBLTMODE(); break;
					case EMR_SETICMMODE:        Read_EMR_SETICMMODE(); break;
					case EMR_SETROP2:           Read_EMR_SETROP2(); break;
					case EMR_REALIZEPALETTE:    Read_EMR_REALIZEPALETTE(); break;
					case EMR_SETLAYOUT:         Read_EMR_SETLAYOUT(); break;
					case EMR_SETBRUSHORGEX:     Read_EMR_SETBRUSHORGEX(); break;
						//-----------------------------------------------------------
						// 2.3.12 Transform
						//-----------------------------------------------------------
					case EMR_SETWORLDTRANSFORM: Read_EMR_SETWORLDTRANSFORM(); break;
					case EMR_MODIFYWORLDTRANSFORM: Read_EMR_MODIFYWORLDTRANSFORM(); break;
						//-----------------------------------------------------------
						// ���������������� ������
						//-----------------------------------------------------------
					case EMR_GDICOMMENT: Read_EMR_UNKNOWN(); break;
						//-----------------------------------------------------------
						// ����������� ������
						//-----------------------------------------------------------
					default:
					{
						std::cout << ulType << " ";
						Read_EMR_UNKNOWN();
						break;
					}
				}

				if (bEof)
					break;

				ulRecordIndex++;

			} while (!CheckError());

			if (!CheckError())
				m_oStream.SeekToStart();

			if (m_pOutput)
				m_pOutput->End();
		}

	private:

		void SetError()
		{
			m_bError = true;
		}
		CEmfDC* GetDC()
		{
			return m_pDC;
		}
		TEmfRectL* GetDCBounds()
		{
			return &m_oHeader.oFrameToBounds;
		}
		bool ReadImage(unsigned int offBmi, unsigned int cbBmi, unsigned int offBits, unsigned int cbBits, unsigned int ulSkip, BYTE** ppBgraBuffer, unsigned int* pulWidth, unsigned int* pulHeight)
		{
			int lHeaderOffset         = offBmi - ulSkip;
			unsigned int ulHeaderSize = cbBmi;
			int lBitsOffset           = offBits - offBmi - cbBmi;
			unsigned int ulBitsSize   = cbBits;
			if (ulHeaderSize <= 0 || ulBitsSize <= 0 || lHeaderOffset < 0 || lBitsOffset < 0)
			{
				// TODO: ���� ������ ����, ������ ���� �������� BitBltRasterOperation
				if (lHeaderOffset > 0)
					m_oStream.Skip(lHeaderOffset);

				m_oStream.Skip(ulHeaderSize);

				if (lBitsOffset > 0)
					m_oStream.Skip(lBitsOffset);

				m_oStream.Skip(ulBitsSize);

				return false;
			}

			m_oStream.Skip(lHeaderOffset);

			BYTE* pHeaderBuffer = new BYTE[ulHeaderSize];
			if (!pHeaderBuffer)
			{
				SetError();
				return false;
			}

			m_oStream.ReadBytes(pHeaderBuffer, ulHeaderSize);

			m_oStream.Skip(lBitsOffset);
			BYTE* pBitsBuffer = new BYTE[ulBitsSize];
			if (!pBitsBuffer)
			{
				delete[] pHeaderBuffer;
				SetError();
				return false;
			}
			m_oStream.ReadBytes(pBitsBuffer, ulBitsSize);

			MetaFile::ReadImage(pHeaderBuffer, ulHeaderSize, pBitsBuffer, ulBitsSize, ppBgraBuffer, pulWidth, pulHeight);

			delete[] pBitsBuffer;
			delete[] pHeaderBuffer;

			return true;
		}
		double TranslateX(int lSrcX)
		{
			double dDstX;

			TEmfWindow* pWindow   = m_pDC->GetWindow();
			TEmfWindow* pViewport = m_pDC->GetViewport();

			dDstX =  (double)((double)(lSrcX - pWindow->lX) * m_pDC->GetPixelWidth()) + pViewport->lX;
			return dDstX;
		}
		double TranslateY(int lSrcY)
		{
			double dDstY;

			TEmfWindow* pWindow   = m_pDC->GetWindow();
			TEmfWindow* pViewport = m_pDC->GetViewport();

			dDstY = (double)((double)(lSrcY - pWindow->lY) * m_pDC->GetPixelHeight()) + pViewport->lY;

			return dDstY;
		}
		double GetEllipseAngle(int nL, int nT, int nR, int nB, int nX, int nY)
		{
			int nX0 = (nL + nR) / 2;
			int nY0 = (nT + nB) / 2;

			// ��������� ��������
			int nQuarter = -1;
			if (nX >= nX0)
			{
				if (nY <= nY0)
					nQuarter = 0;
				else
					nQuarter = 3;
			}
			else
			{
				if (nY <= nY0)
					nQuarter = 1;
				else
					nQuarter = 2;
			}

			double dDist = std::sqrt((nX - nX0) * (nX - nX0) + (nY - nY0) * (nY - nY0));
			double dRadAngle = std::asin(std::abs(nY - nY0) / dDist);
			
			double dAngle = dRadAngle * 180 / 3.1415926;
			switch (nQuarter)
			{
				case 1: dAngle = 180 - dAngle; break;
				case 2: dAngle = 180 + dAngle; break;
				case 3: dAngle = 360 - dAngle; break;
			}

			return dAngle;
		}

		void MoveTo(TEmfPointL& oPoint)
		{
			MoveTo(oPoint.x, oPoint.y);
		}
		void MoveTo(TEmfPointS& oPoint)
		{
			MoveTo(oPoint.x, oPoint.y);
		}
		void MoveTo(int lX, int lY)
		{
			if (m_pPath)
			{
				if (!m_pPath->MoveTo(lX, lY))
					return SetError();
			}
			else if (m_pOutput)
			{
				m_pOutput->MoveTo(lX, lY);
			}

			m_pDC->SetCurPos(lX, lY);
		}
		void LineTo(int lX, int lY)
		{
			if (m_pPath)
			{
				if (!m_pPath->LineTo(lX, lY))
					return SetError();
			}
			else if (m_pOutput)
			{
				m_pOutput->LineTo(lX, lY);
			}

			m_pDC->SetCurPos(lX, lY);
		}
		void LineTo(TEmfPointL& oPoint)
		{
			LineTo(oPoint.x, oPoint.y);
		}
		void LineTo(TEmfPointS& oPoint)
		{
			LineTo(oPoint.x, oPoint.y);
		}
		void CurveTo(int nX1, int nY1, int nX2, int nY2, int nXe, int nYe)
		{
			if (m_pPath)
			{
				if (!m_pPath->CurveTo(nX1, nY1, nX2, nY2, nXe, nYe))
					return SetError();
			}
			else if (m_pOutput)
			{
				m_pOutput->CurveTo(nX1, nY1, nX2, nY2, nXe, nYe);
			}

			m_pDC->SetCurPos(nXe, nYe);
		}
		void CurveTo(TEmfPointS& oPoint1, TEmfPointS& oPoint2, TEmfPointS& oPointE)
		{
			CurveTo(oPoint1.x, oPoint1.y, oPoint2.x, oPoint2.y, oPointE.x, oPointE.y);
		}
		void CurveTo(TEmfPointL& oPoint1, TEmfPointL& oPoint2, TEmfPointL& oPointE)
		{
			CurveTo(oPoint1.x, oPoint1.y, oPoint2.x, oPoint2.y, oPointE.x, oPointE.y);
		}
		void ClosePath()
		{
			if (m_pPath)
			{
				if (!m_pPath->Close())
					return SetError();
			}
			else if (m_pOutput)
				m_pOutput->ClosePath();
		}
		void ArcTo(int lL, int lT, int lR, int lB, double dStart, double dSweep)
		{
			if (m_pPath)
			{
				if (!m_pPath->ArcTo(lL, lT, lR, lB, dStart, dSweep))
					return SetError();
			}
			else if (m_pOutput)
			{
				m_pOutput->ArcTo(lL, lT, lR, lB, dStart, dSweep);
			}

			// TODO: ������� �������� ������� �������
		}
		void DrawPath(bool bStroke, bool bFill)
		{
			if (m_pPath && m_pOutput)
			{
				//m_pPath->Draw(m_pOutput, bStroke, bFill);
			}
			else if (m_pOutput)
			{
				int lType = (bStroke ? 1 : 0) + (bFill ? 2 : 0);
				m_pOutput->DrawPath(lType);
				m_pOutput->EndPath();
			}
		}
		void UpdateOutputDC()
		{
			if (m_pOutput)
				m_pOutput->UpdateDC();
		}
		void DrawText(std::wstring& wsString, unsigned int unCharsCount, int _nX, int _nY)
		{
			int nX = _nX;
			int nY = _nY;

			if (m_pDC->GetTextAlign() & TA_UPDATECP)
			{
				nX = m_pDC->GetCurPos().x;
				nY = m_pDC->GetCurPos().y;
			}
			else
			{
				CEmfLogFont* pFont = m_pDC->GetFont();
				if (pFont && pFont->LogFontEx.LogFont.Height < 0)
				{
					// �������� ����� ������ ����� �� �������� � � ������ TA_UPDATECP.
					//nY -= pFont->LogFontEx.LogFont.Height;
				}
			}

			if (m_pOutput)
				m_pOutput->DrawText(wsString.c_str(), unCharsCount, nX, nY);
		}
		void DrawTextA(TEmfEmrText& oText)
		{
			if (!oText.OutputString)
				return SetError();

			// TODO: ����� ����� ������� ������� ������ �������� � Unicode.
			std::wstring wsText;
			for (unsigned int unIndex = 0; unIndex < oText.Chars; unIndex++)
			{
				wchar_t wUnicode = ((unsigned char*)oText.OutputString)[unIndex];
				wsText += wUnicode;
			}

			DrawText(wsText, oText.Chars, oText.Reference.x, oText.Reference.y);
		}
		void DrawTextW(TEmfEmrText& oText)
		{
			if (!oText.OutputString)
				return SetError();

			std::wstring wsText((wchar_t*)oText.OutputString);

			DrawText(wsText, oText.Chars, oText.Reference.x, oText.Reference.y);
		}

		void Read_EMR_HEADER()
		{
			m_oStream >> m_oHeader.oBounds;
			m_oStream >> m_oHeader.oFrame;
			m_oStream >> m_oHeader.ulSignature;
			m_oStream >> m_oHeader.ulVersion;
			m_oStream >> m_oHeader.ulSize;
			m_oStream >> m_oHeader.ulRecords;
			m_oStream >> m_oHeader.ushObjects;
			m_oStream >> m_oHeader.ushReserved;
			m_oStream >> m_oHeader.ulSizeDescription;
			m_oStream >> m_oHeader.ulOffsetDescription;
			m_oStream >> m_oHeader.ulPalEntries;
			m_oStream >> m_oHeader.oDevice;
			m_oStream >> m_oHeader.oMillimeters;

			if (ENHMETA_SIGNATURE != m_oHeader.ulSignature || 0x00010000 != m_oHeader.ulVersion)
				return SetError();

			// ���������� ��������� ����� ���������, �.�. ��� ��� ���� �� ����������
			unsigned int ulRemaining = m_ulRecordSize - 80; // sizeof(TEmfHeader)
			m_oStream.Skip(ulRemaining);

			double dL = m_oHeader.oFrame.lLeft / 100.0 / m_oHeader.oMillimeters.cx * m_oHeader.oDevice.cx;
			double dR = m_oHeader.oFrame.lRight / 100.0 / m_oHeader.oMillimeters.cx * m_oHeader.oDevice.cx;
			double dT = m_oHeader.oFrame.lTop / 100.0 / m_oHeader.oMillimeters.cy * m_oHeader.oDevice.cy;
			double dB = m_oHeader.oFrame.lBottom / 100.0 / m_oHeader.oMillimeters.cy * m_oHeader.oDevice.cy;

			double dW = dR - dL;
			double dH = dB - dT;

			int lL = (int)floor(dL + 0.5);
			int lT = (int)floor(dT + 0.5);
			int lR = (int)floor(dW + 0.5) + lL;
			int lB = (int)floor(dH + 0.5) + lT;

			// �� ������ �� ������ �������� ����, ����� ����� �� ��� � oBounds, �� ���� �����, ��� ��� �� ���.
			m_oHeader.oFrameToBounds.lLeft   = lL;
			m_oHeader.oFrameToBounds.lRight  = lR;
			m_oHeader.oFrameToBounds.lTop    = lT;
			m_oHeader.oFrameToBounds.lBottom = lB;
		}
		void Read_EMR_STRETCHDIBITS()
		{
			TEmfStretchDIBITS oBitmap;
			m_oStream >> oBitmap;

			BYTE* pBgraBuffer = NULL;
			unsigned int ulWidth, ulHeight;

			if (ReadImage(oBitmap.offBmiSrc, oBitmap.cbBmiSrc, oBitmap.offBitsSrc, oBitmap.cbBitsSrc, sizeof(TEmfStretchDIBITS) + 8, &pBgraBuffer, &ulWidth, &ulHeight))
			{
				// ��� ������� �������� SRCPAINT � SRCAND �������, ��� ����� ��� ����� �����.
				if (0x008800C6 == oBitmap.BitBltRasterOperation) // SRCPAINT
				{
					BYTE* pCur = pBgraBuffer;
					for (unsigned int unY = 0; unY < ulHeight; unY++)
					{
						for (unsigned int unX = 0; unX < ulWidth; unX++)
						{
							unsigned int unIndex = (unY * ulWidth + unX) * 4;

							if (0xff == pCur[unIndex + 0] && 0xff == pCur[unIndex + 1] && 0xff == pCur[unIndex + 2])
								pCur[unIndex + 3] = 0;
						}
					}
				}
				else if (0x00EE0086 == oBitmap.BitBltRasterOperation) // SRCAND
				{
					BYTE* pCur = pBgraBuffer;
					for (unsigned int unY = 0; unY < ulHeight; unY++)
					{
						for (unsigned int unX = 0; unX < ulWidth; unX++)
						{
							unsigned int unIndex = (unY * ulWidth + unX) * 4;

							if (0 == pCur[unIndex + 0] && 0 == pCur[unIndex + 1] && 0 == pCur[unIndex + 2])
								pCur[unIndex + 3] = 0;
						}
					}
				}

				if (m_pOutput)
					m_pOutput->DrawBitmap(oBitmap.xDest, oBitmap.yDest, oBitmap.cxDest, oBitmap.cyDest, pBgraBuffer, ulWidth, ulHeight);
			}

			if (pBgraBuffer)
				delete[] pBgraBuffer;
		}
		void Read_EMR_BITBLT()
		{
			TEmfBitBlt oBitmap;
			m_oStream >> oBitmap;

			BYTE* pBgraBuffer = NULL;
			unsigned int ulWidth, ulHeight;

			if (ReadImage(oBitmap.offBmiSrc, oBitmap.cbBmiSrc, oBitmap.offBitsSrc, oBitmap.cbBitsSrc, sizeof(TEmfBitBlt) + 8, &pBgraBuffer, &ulWidth, &ulHeight))
			{
				if (m_pOutput)
					m_pOutput->DrawBitmap(oBitmap.xDest, oBitmap.yDest, oBitmap.cxDest, oBitmap.cyDest, pBgraBuffer, ulWidth, ulHeight);
			}

			if (m_pOutput)
			{
				if (0x00000042 == oBitmap.BitBltRasterOperation) // BLACKNESS
				{
					// ������ ��� ������ ������
					pBgraBuffer = new BYTE[4];
					pBgraBuffer[0] = 0x00;
					pBgraBuffer[1] = 0x00;
					pBgraBuffer[2] = 0x00;
					pBgraBuffer[3] = 0xff;

					ulWidth  = 1;
					ulHeight = 1;
				}
				if (0x00FF0062 == oBitmap.BitBltRasterOperation) // WHITENESS
				{
					// ������ ��� ������ ������
					pBgraBuffer = new BYTE[4];
					pBgraBuffer[0] = 0xff;
					pBgraBuffer[1] = 0xff;
					pBgraBuffer[2] = 0xff;
					pBgraBuffer[3] = 0xff;

					ulWidth  = 1;
					ulHeight = 1;
				}
				else if (0x00f00021 == oBitmap.BitBltRasterOperation) // PATCOPY
				{
					CEmfLogBrushEx* pBrush = m_pDC->GetBrush();
					if (pBrush)
					{					
						// ������ ������ �����
						pBgraBuffer = new BYTE[4];
						pBgraBuffer[0] = pBrush->Color.b;
						pBgraBuffer[1] = pBrush->Color.g;
						pBgraBuffer[2] = pBrush->Color.r;
						pBgraBuffer[3] = 0xff;

						ulWidth  = 1;
						ulHeight = 1;
					}
				}
				else if (0x005a0049 == oBitmap.BitBltRasterOperation) // PATINVERT
				{
					CEmfLogBrushEx* pBrush = m_pDC->GetBrush();
					if (pBrush)
					{
						// ������ ������ �����
						pBgraBuffer = new BYTE[4];
						pBgraBuffer[0] = pBrush->Color.b;
						pBgraBuffer[1] = pBrush->Color.g;
						pBgraBuffer[2] = pBrush->Color.r;
						pBgraBuffer[3] = 30;

						ulWidth  = 1;
						ulHeight = 1;
					}
				}
				else if (0x00A000C9 == oBitmap.BitBltRasterOperation) // PATINVERT
				{
					CEmfLogBrushEx* pBrush = m_pDC->GetBrush();
					if (pBrush)
					{
						// ������ ������ �����
						pBgraBuffer = new BYTE[4];
						pBgraBuffer[0] = pBrush->Color.b;
						pBgraBuffer[1] = pBrush->Color.g;
						pBgraBuffer[2] = pBrush->Color.r;
						pBgraBuffer[3] = 30;

						ulWidth  = 1;
						ulHeight = 1;
					}
				}

				if (pBgraBuffer)
					m_pOutput->DrawBitmap(oBitmap.xDest, oBitmap.yDest, oBitmap.cxDest, oBitmap.cyDest, pBgraBuffer, ulWidth, ulHeight);				
			}

			if (pBgraBuffer)
				delete[] pBgraBuffer;
		}
		void Read_EMR_SETDIBITSTODEVICE()
		{
			TEmfSetDiBitsToDevice oBitmap;
			m_oStream >> oBitmap;

			BYTE* pBgraBuffer = NULL;
			unsigned int ulWidth, ulHeight;
			if (ReadImage(oBitmap.offBmiSrc, oBitmap.cbBmiSrc, oBitmap.offBitsSrc, oBitmap.cbBitsSrc, sizeof(TEmfSetDiBitsToDevice) + 8, &pBgraBuffer, &ulWidth, &ulHeight))
			{
				// TODO: ����� ����������� ������� �������� �� ���������� oBitmap.iStartScan � oBitmap.cScans
				if (m_pOutput)
					m_pOutput->DrawBitmap(oBitmap.Bounds.lLeft, oBitmap.Bounds.lTop, oBitmap.Bounds.lRight - oBitmap.Bounds.lLeft, oBitmap.Bounds.lBottom - oBitmap.Bounds.lTop, pBgraBuffer, ulWidth, ulHeight);
			}

			if (pBgraBuffer)
				delete[] pBgraBuffer;
		}
		void Read_EMR_EOF()
		{
			unsigned int ulCount, ulOffset, ulSizeLast;

			m_oStream >> ulCount;
			m_oStream >> ulOffset;

			m_oStream.Skip(m_ulRecordSize - 8 - 4);

			m_oStream >> ulSizeLast;
		}
		void Read_EMR_UNKNOWN()
		{
			// ����������� � ��������������� ������ �� ����������
			m_oStream.Skip(m_ulRecordSize);
		}
		void Read_EMR_SAVEDC()
		{
			m_pDC = m_oPlayer.SaveDC();
		}
		void Read_EMR_RESTOREDC()
		{
			int lSavedDC;
			m_oStream >> lSavedDC;

			if (lSavedDC >= 0)
			{
				SetError();
				return;
			}

			int lCount = -lSavedDC;
			for (int lIndex = 0; lIndex < lCount; lIndex++)
				m_oPlayer.RestoreDC();

			m_pDC = m_oPlayer.GetDC();
			UpdateOutputDC();
		}
		void Read_EMR_MODIFYWORLDTRANSFORM()
		{
			TEmfXForm oXForm;
			unsigned int ulMode;

			m_oStream >> oXForm;
			m_oStream >> ulMode;

			m_pDC->MultiplyTransform(oXForm, ulMode);
			UpdateOutputDC();
		}
		void Read_EMR_SETWORLDTRANSFORM()
		{
			TEmfXForm oXForm;

			m_oStream >> oXForm;

			m_pDC->MultiplyTransform(oXForm, MWT_SET);
			UpdateOutputDC();
		}
		void Read_EMR_CREATEBRUSHINDIRECT()
		{
			unsigned int ulBrushIndex;
			CEmfLogBrushEx* pBrush = new CEmfLogBrushEx();
			if (!pBrush)
				return SetError();

			m_oStream >> ulBrushIndex;
			m_oStream >> *pBrush;

			m_oPlayer.RegisterObject(ulBrushIndex, (CEmfObjectBase*)pBrush);
		}
		void Read_EMR_SETTEXTCOLOR()
		{
			TEmfColor oColor;
			m_oStream >> oColor;

			m_pDC->SetTextColor(oColor);
			UpdateOutputDC();
		}		
		void Read_EMR_SELECTOBJECT()
		{
			unsigned int ulObjectIndex;
			m_oStream >> ulObjectIndex;

			m_oPlayer.SelectObject(ulObjectIndex);
			UpdateOutputDC();
		}
		void Read_EMR_EXTCREATEFONTINDIRECTW()
		{
			unsigned int ulIndex;
			CEmfLogFont* pFont = new CEmfLogFont();
			if (!pFont)
				return SetError();

			m_oStream >> ulIndex;
			m_oStream >> *pFont;
			m_oPlayer.RegisterObject(ulIndex, (CEmfObjectBase*)pFont);
		}
		void Read_EMR_SETTEXTALIGN()
		{
			unsigned int ulAlign;
			m_oStream >> ulAlign;

			m_pDC->SetTextAlign(ulAlign);
			UpdateOutputDC();
		}
		void Read_EMR_SETBKMODE()
		{
			unsigned int ulBgMode;
			m_oStream >> ulBgMode;
			m_pDC->SetBgMode(ulBgMode);
			UpdateOutputDC();
		}
		void Read_EMR_DELETEOBJECT()
		{
			unsigned int ulIndex;
			m_oStream >> ulIndex;
			m_oPlayer.DeleteObject(ulIndex);
			UpdateOutputDC();
		}
		void Read_EMR_SETMITERLIMIT()
		{
			unsigned int ulMiterLimit;
			m_oStream >> ulMiterLimit;
			m_pDC->SetMiterLimit(ulMiterLimit);
			UpdateOutputDC();
		}
		void Read_EMR_EXTCREATEPEN()
		{
			unsigned int ulPenIndex;
			m_oStream >> ulPenIndex;

			m_oStream.Skip(4); // offBmi
			m_oStream.Skip(4); // cbBmi
			m_oStream.Skip(4); // offBits
			m_oStream.Skip(4); // cbBits

			m_ulRecordSize -= 20;

			CEmfLogPen* pPen = new CEmfLogPen();
			if (!pPen)
				return SetError();

			// LogPenEx
			m_oStream >> pPen->PenStyle;
			m_oStream >> pPen->Width;
			m_oStream.Skip(4); // BrushStyle
			m_oStream >> pPen->Color;
			m_oStream.Skip(4); // BrushHatch
			m_oStream >> pPen->NumStyleEntries;

			m_ulRecordSize -= 24;
			if (pPen->NumStyleEntries > 0)
			{
				m_ulRecordSize -= pPen->NumStyleEntries * 4;
				pPen->StyleEntry = new unsigned int[pPen->NumStyleEntries];
				if (!pPen->StyleEntry)
				{
					delete pPen;
					return SetError();
				}

				for (unsigned int ulIndex = 0; ulIndex < pPen->NumStyleEntries; ulIndex++)
				{
					m_oStream >> pPen->StyleEntry[ulIndex];
				}
			}
			else
			{
				pPen->StyleEntry = NULL;
			}

			// ���������� ����� � ���������, ���� ��� ����
			m_oStream.Skip(m_ulRecordSize);

			m_oPlayer.RegisterObject(ulPenIndex, (CEmfObjectBase*)pPen);
		}
		void Read_EMR_CREATEPEN()
		{
			unsigned int ulPenIndex;
			m_oStream >> ulPenIndex;
			CEmfLogPen* pPen = new CEmfLogPen();
			if (!pPen)
				return SetError();

			m_oStream >> pPen->PenStyle;
			m_oStream >> pPen->Width;
			m_oStream.Skip(4); // Width.y
			m_oStream >> pPen->Color;
			m_oPlayer.RegisterObject(ulPenIndex, (CEmfObjectBase*)pPen);
		}
		void Read_EMR_SETPOLYFILLMODE()
		{
			unsigned int ulFillMode;
			m_oStream >> ulFillMode;
			m_pDC->SetFillMode(ulFillMode);
			UpdateOutputDC();
		}
		void Read_EMR_BEGINPATH()
		{
			if (m_pPath)
				delete m_pPath;

			m_pPath = new CEmfPath();
			if (!m_pPath)
				SetError();

			// ������ MoveTo ���� �� BeginPath
			m_pPath->MoveTo(m_pDC->GetCurPos());
		}
		void Read_EMR_ENDPATH()
		{
			// ������ �� ������
		}
		void Read_EMR_CLOSEFIGURE()
		{
			if (m_pPath)
			{
				if (!m_pPath->Close())
					return SetError();
			}
		}
		void Read_EMR_FLATTENPATH()
		{
			// ������ �� ������
		}
		void Read_EMR_WIDENPATH()
		{
			// TODO: �����������
		}
		void Read_EMR_ABORTPATH()
		{
			if (m_pPath)
			{
				delete m_pPath;
				m_pPath = NULL;
			}
		}
		void Read_EMR_MOVETOEX()
		{
			TEmfPointL oPoint;
			m_oStream >> oPoint;
			MoveTo(oPoint);
		}		
		void Read_EMR_FILLPATH()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;

			if (m_pPath)
			{
				m_pPath->Draw(m_pOutput, false, true);
				RELEASEOBJECT(m_pPath);
			}
		}
		void Read_EMR_SETMAPMODE()
		{
			unsigned int ulMapMode;
			m_oStream >> ulMapMode;

			m_pDC->SetMapMode(ulMapMode);
		}
		void Read_EMR_SETWINDOWORGEX()
		{
			TEmfPointL oOrigin;
			m_oStream >> oOrigin;
			m_pDC->SetWindowOrigin(oOrigin);
		}
		void Read_EMR_SETWINDOWEXTEX()
		{
			TEmfSizeL oExtent;
			m_oStream >> oExtent;
			m_pDC->SetWindowExtents(oExtent);
		}
		void Read_EMR_SETVIEWPORTORGEX()
		{
			TEmfPointL oOrigin;
			m_oStream >> oOrigin;
			m_pDC->SetViewportOrigin(oOrigin);
		}
		void Read_EMR_SETVIEWPORTEXTEX()
		{
			TEmfSizeL oExtent;
			m_oStream >> oExtent;
			m_pDC->SetViewportExtents(oExtent);
		}
		void Read_EMR_SETSTRETCHBLTMODE()
		{
			unsigned int ulStretchMode;
			m_oStream >> ulStretchMode;
			m_pDC->SetStretchMode(ulStretchMode);
		}
		void Read_EMR_SETICMMODE()
		{
			unsigned int ulICMMode;
			m_oStream >> ulICMMode;
		}
		void Read_EMR_CREATEDIBPATTERNBRUSHPT()
		{
			unsigned int ulBrushIndex;
			TEmfDibPatternBrush oDibBrush;
			m_oStream >> ulBrushIndex;
			m_oStream >> oDibBrush;

			BYTE* pBgraBuffer = NULL;
			unsigned int ulWidth, ulHeight;

			if (ReadImage(oDibBrush.offBmi, oDibBrush.cbBmi, oDibBrush.offBits, oDibBrush.cbBits, sizeof(TEmfDibPatternBrush) + 12, &pBgraBuffer, &ulWidth, &ulHeight))
			{
				CEmfLogBrushEx* pBrush = new CEmfLogBrushEx();
				if (!pBrush)
					return SetError();

				pBrush->SetDibPattern(pBgraBuffer, ulWidth, ulHeight);
				m_oPlayer.RegisterObject(ulBrushIndex, (CEmfObjectBase*)pBrush);
			}
		}
		void Read_EMR_SELECTCLIPPATH()
		{
			unsigned int unRegionMode;
			m_oStream >> unRegionMode;

			if (m_pPath)
			{
				m_pDC->ClipToPath(m_pPath, unRegionMode);
				RELEASEOBJECT(m_pPath);

				UpdateOutputDC();
			}
		}
		void Read_EMR_SETBKCOLOR()
		{
			TEmfColor oColor;
			m_oStream >> oColor;
			// TODO: �����������

			UpdateOutputDC();
		}
		void Read_EMR_EXTSELECTCLIPRGN()
		{
			unsigned int ulRgnDataSize, ulRegionMode;
			m_oStream >> ulRgnDataSize >> ulRegionMode;

			m_oStream.Skip(m_ulRecordSize - 8);
			// TODO: ����������� ����
		}
		void Read_EMR_SETMETARGN()
		{
			m_pDC->GetClip()->Reset();
			UpdateOutputDC();
		}		
		void Read_EMR_SETROP2()
		{
			unsigned int ulRop2Mode;
			m_oStream >> ulRop2Mode;
			m_pDC->SetRop2Mode(ulRop2Mode);
			UpdateOutputDC();
		}
		void Read_EMR_CREATEPALETTE()
		{
			unsigned int ulPaletteIndex;
			CEmfLogPalette* pPalette = new CEmfLogPalette();
			if (!pPalette)
				return SetError();

			m_oStream >> ulPaletteIndex;
			m_oStream >> *pPalette;
			m_oPlayer.RegisterObject(ulPaletteIndex, (CEmfObjectBase*)pPalette);
		}
		void Read_EMR_SELECTPALETTE()
		{
			unsigned int ulIndex;
			m_oStream >> ulIndex;
			m_oPlayer.SelectPalette(ulIndex);
		}
		void Read_EMR_REALIZEPALETTE()
		{
			// TODO: �����������
		}
		void Read_EMR_INTERSECTCLIPRECT()
		{
			TEmfRectL oClip;
			m_oStream >> oClip;
			m_pDC->GetClip()->Intersect(oClip);
		}
		void Read_EMR_SETLAYOUT()
		{
			unsigned int ulLayoutMode;
			m_oStream >> ulLayoutMode;

			// TODO: �����������
		}
		void Read_EMR_SETBRUSHORGEX()
		{
			TEmfPointL oOrigin;
			m_oStream >> oOrigin;

			// TODO: �����������
		}		
		void Read_EMR_ANGLEARC()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfPointL oCenter;
			unsigned int unRadius;
			double dStartAngle, dSweepAngle;
			m_oStream >> oCenter >> unRadius >> dStartAngle >> dSweepAngle;

			ArcTo(oCenter.x - unRadius, oCenter.y - unRadius, oCenter.x + unRadius, oCenter.y + unRadius, dStartAngle, dSweepAngle);
			DrawPath(true, false);
		}
		void Read_EMR_ARC_BASE(TEmfRectL& oBox, TEmfPointL& oStart, TEmfPointL& oEnd, double& dStartAngle, double& dSweep)
		{
			m_oStream >> oBox >> oStart >> oEnd;

			int nX0 = (oBox.lRight + oBox.lLeft) / 2;
			int nY0 = (oBox.lBottom + oBox.lTop) / 2;

			dStartAngle = GetEllipseAngle(oBox.lLeft, oBox.lTop, oBox.lRight, oBox.lBottom, oStart.x, oStart.y);
			double dEndAngle   = GetEllipseAngle(oBox.lLeft, oBox.lTop, oBox.lRight, oBox.lBottom, oEnd.x, oEnd.y);

			dSweep = dEndAngle - dStartAngle;

			// TODO: ��������� �����
			if (dSweep < 0)
				dSweep += 360;
		}
		void Read_EMR_ARC()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfRectL oBox;
			TEmfPointL oStart, oEnd;
			double dStartAngle, dSweep;
			Read_EMR_ARC_BASE(oBox, oStart, oEnd, dStartAngle, dSweep);

			ArcTo(oBox.lLeft, oBox.lTop, oBox.lRight, oBox.lBottom, dStartAngle, dSweep);
			DrawPath(true, false);
		}
		void Read_EMR_ARCTO()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfRectL oBox;
			TEmfPointL oStart, oEnd;
			double dStartAngle, dSweep;
			Read_EMR_ARC_BASE(oBox, oStart, oEnd, dStartAngle, dSweep);

			ArcTo(oBox.lLeft, oBox.lTop, oBox.lRight, oBox.lBottom, dStartAngle, dSweep);
		}
		void Read_EMR_CHORD()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfRectL oBox;
			TEmfPointL oStart, oEnd;
			double dStartAngle, dSweep;
			Read_EMR_ARC_BASE(oBox, oStart, oEnd, dStartAngle, dSweep);

			MoveTo(oStart);
			ArcTo(oBox.lLeft, oBox.lTop, oBox.lRight, oBox.lBottom, dStartAngle, dSweep);
			LineTo(oStart);
			DrawPath(true, true);
		}
		void Read_EMR_ELLIPSE()
		{
			TEmfRectL oBox;
			m_oStream >> oBox;
			ArcTo(oBox.lLeft, oBox.lTop, oBox.lRight, oBox.lBottom, 0, 360);
			DrawPath(true, true);
		}
		void Read_EMR_EXTTEXTOUTA()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfExtTextoutA oText;
			m_oStream >> oText;	

			DrawTextA(oText.aEmrText);
		}
		void Read_EMR_EXTTEXTOUTW()
		{
			TEmfExtTextoutW oText;
			m_oStream >> oText;

			DrawTextW(oText.wEmrText);
		}
		void Read_EMR_LINETO()
		{
			TEmfPointL oPoint;
			m_oStream >> oPoint;
			LineTo(oPoint);
		}
		void Read_EMR_PIE()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfRectL oBox;
			TEmfPointL oStart, oEnd;
			double dStartAngle, dSweep;
			Read_EMR_ARC_BASE(oBox, oStart, oEnd, dStartAngle, dSweep);

			ArcTo(oBox.lLeft, oBox.lTop, oBox.lRight, oBox.lBottom, dStartAngle, dSweep);
			LineTo((oBox.lLeft + oBox.lRight) / 2, (oBox.lTop + oBox.lBottom) / 2);
			ClosePath();
			DrawPath(true, true);
		}
		void Read_EMR_POLYBEZIER()
		{
			Read_EMR_POLYBEZIER_BASE<TEmfPointL>();
		}
		void Read_EMR_POLYBEZIER16()
		{
			Read_EMR_POLYBEZIER_BASE<TEmfPointS>();
		}
		template<typename T>void Read_EMR_POLYBEZIER_BASE()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;

			unsigned int ulCount;
			m_oStream >> ulCount;

			if (0 == ulCount)
				return;

			T oStartPoint;
			m_oStream >> oStartPoint;
			MoveTo(oStartPoint);

			T oPoint1, oPoint2, oPointE;
			for (unsigned int ulIndex = 1; ulIndex < ulCount; ulIndex += 3)
			{
				m_oStream >> oPoint1 >> oPoint2 >> oPointE;
				CurveTo(oPoint1, oPoint2, oPointE);
			}
			DrawPath(true, false);
		}
		void Read_EMR_POLYBEZIERTO()
		{
			Read_EMR_POLYBEZIERTO_BASE<TEmfPointL>();
		}
		void Read_EMR_POLYBEZIERTO16()
		{
			Read_EMR_POLYBEZIERTO_BASE<TEmfPointS>();
		}
		template<typename T>void Read_EMR_POLYBEZIERTO_BASE()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;

			unsigned int ulCount;
			m_oStream >> ulCount;

			for (unsigned int ulIndex = 0; ulIndex < ulCount; ulIndex += 3)
			{
				if (ulCount - ulIndex < 2)
					return SetError();

				T oPoint1, oPoint2, oPointE;
				m_oStream >> oPoint1 >> oPoint2 >> oPointE;
				CurveTo(oPoint1, oPoint2, oPointE);
			}
		}
		void Read_EMR_POLYDRAW()
		{
			Read_EMR_POLYDRAW_BASE<TEmfPointL>();
		}
		void Read_EMR_POLYDRAW16()
		{
			Read_EMR_POLYDRAW_BASE<TEmfPointS>();
		}
		template<typename T>void Read_EMR_POLYDRAW_BASE()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfRectL oBounds;
			m_oStream >> oBounds;

			unsigned int unCount;
			m_oStream >> unCount;

			if (0 == unCount)
				return;

			T* pPoints = new T[unCount];
			if (!pPoints)
				return SetError();

			for (unsigned int unIndex = 0; unIndex < unCount; unIndex++)
			{
				m_oStream >> pPoints[unIndex];
			}

			unsigned char* pAbTypes = new unsigned char[unCount];
			if (!pAbTypes)
			{
				delete[] pPoints;
				return SetError();
			}

			for (unsigned int unIndex = 0; unIndex < unCount; unIndex++)
			{
				m_oStream >> pAbTypes[unIndex];
			}

			T* pPoint1 = NULL, *pPoint2 = NULL;
			for (unsigned int unIndex = 0, unPointIndex = 0; unIndex < unCount; unIndex++)
			{
				unsigned char unType = pAbTypes[unIndex];
				T* pPoint = pPoints + unIndex;
				if (PT_MOVETO == unType)
				{
					MoveTo(*pPoint);
					unPointIndex = 0;
				}
				else if (PT_LINETO & unType)
				{
					LineTo(*pPoint);
					if (PT_CLOSEFIGURE & unType)
						ClosePath();
					unPointIndex = 0;
				}
				else if (PT_BEZIERTO & unType)
				{
					if (0 == unPointIndex)
					{
						pPoint1 = pPoint;
						unPointIndex = 1;
					}
					else if (1 == unPointIndex)
					{
						pPoint2 = pPoint;
						unPointIndex = 2;
					}
					else if (2 == unPointIndex)
					{
						CurveTo(*pPoint1, *pPoint2, *pPoint);
						unPointIndex = 0;

						if (PT_CLOSEFIGURE & unType)
							ClosePath();
					}
					else
					{
						SetError();
						break;
					}
				}

			}

			delete[] pPoints;
			delete[] pAbTypes;
		}
		void Read_EMR_POLYGON()
		{
			Read_EMR_POLYGON_BASE<TEmfPointL>();
		}
		void Read_EMR_POLYGON16()
		{
			Read_EMR_POLYGON_BASE<TEmfPointS>();
		}
		template<typename T>void Read_EMR_POLYGON_BASE()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;
			unsigned int ulCount;
			m_oStream >> ulCount;

			if (ulCount <= 0)
				return;

			T oPoint;
			m_oStream >> oPoint;
			MoveTo(oPoint);
			for (unsigned int ulIndex = 1; ulIndex < ulCount; ulIndex++)
			{
				m_oStream >> oPoint;
				LineTo(oPoint);
			}
			ClosePath();
			DrawPath(true, true);
		}
		void Read_EMR_POLYLINE()
		{
			Read_EMR_POLYLINE_BASE<TEmfPointL>();
		}
		void Read_EMR_POLYLINE16()
		{
			Read_EMR_POLYLINE_BASE<TEmfPointS>();
		}
		template<typename T>void Read_EMR_POLYLINE_BASE()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;
			unsigned int ulCount;
			m_oStream >> ulCount;

			if (0 == ulCount)
				return;

			T oPoint;
			m_oStream >> oPoint;
			MoveTo(oPoint);

			for (unsigned int ulIndex = 1; ulIndex < ulCount; ulIndex++)
			{
				m_oStream >> oPoint;
				LineTo(oPoint);
			}

			DrawPath(true, false);
		}
		void Read_EMR_POLYLINETO()
		{
			Read_EMR_POLYLINETO_BASE<TEmfPointL>();
		}
		void Read_EMR_POLYLINETO16()
		{
			Read_EMR_POLYLINETO_BASE<TEmfPointS>();
		}
		template<typename T>void Read_EMR_POLYLINETO_BASE()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;

			unsigned int ulCount;
			m_oStream >> ulCount;

			for (unsigned int ulIndex = 0; ulIndex < ulCount; ulIndex++)
			{
				T oPoint;
				m_oStream >> oPoint;
				LineTo(oPoint);
			}
		}
		void Read_EMR_POLYPOLYGON()
		{
			Read_EMR_POLYPOLYGON_BASE<TEmfPointL>();
		}
		void Read_EMR_POLYPOLYGON16()
		{
			Read_EMR_POLYPOLYGON_BASE<TEmfPointS>();			
		}
		template<typename T>void Read_EMR_POLYPOLYGON_BASE()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;
			unsigned int ulNumberOfPolygons;
			m_oStream >> ulNumberOfPolygons;
			unsigned int ulTotalPointsCount;
			m_oStream >> ulTotalPointsCount;

			unsigned int* pPolygonPointCount = new unsigned int[ulNumberOfPolygons];
			if (!pPolygonPointCount)
				return SetError();

			for (unsigned int ulIndex = 0; ulIndex < ulNumberOfPolygons; ulIndex++)
			{
				m_oStream >> pPolygonPointCount[ulIndex];
			}

			for (unsigned int ulPolygonIndex = 0, unStartPointIndex = 0; ulPolygonIndex < ulNumberOfPolygons; ulPolygonIndex++)
			{
				unsigned int ulCurrentPolygonPointsCount = pPolygonPointCount[ulPolygonIndex];
				if (0 == ulCurrentPolygonPointsCount)
					continue;

				T oPoint;
				m_oStream >> oPoint;
				MoveTo(oPoint);

				for (unsigned int ulPointIndex = 1; ulPointIndex < ulCurrentPolygonPointsCount; ulPointIndex++)
				{
					unsigned int ulRealPointIndex = ulPointIndex + unStartPointIndex;
					if (ulRealPointIndex >= ulTotalPointsCount)
					{
						delete[] pPolygonPointCount;
						return SetError();
					}

					m_oStream >> oPoint;
					LineTo(oPoint);
				}

				ClosePath();
			}
			DrawPath(true, true);

			delete[] pPolygonPointCount;
		}
		void Read_EMR_POLYPOLYLINE()
		{
			Read_EMR_POLYPOLYLINE_BASE<TEmfPointL>();
		}
		void Read_EMR_POLYPOLYLINE16()
		{
			Read_EMR_POLYPOLYLINE_BASE<TEmfPointS>();
		}
		template<typename T>void Read_EMR_POLYPOLYLINE_BASE()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;

			unsigned int ulNumberOfPolylines;
			m_oStream >> ulNumberOfPolylines;

			unsigned int ulTotalPointsCount;
			m_oStream >> ulTotalPointsCount;

			if (0 == ulNumberOfPolylines && 0 == ulTotalPointsCount)
				return;
			else if (0 == ulNumberOfPolylines || 0 == ulTotalPointsCount)
				return SetError();

			unsigned int* pPolylinePointCount = new unsigned int[ulNumberOfPolylines];
			for (unsigned int ulIndex = 0; ulIndex < ulNumberOfPolylines; ulIndex++)
			{
				m_oStream >> pPolylinePointCount[ulIndex];
			}

			for (unsigned int ulPolyIndex = 0, ulStartPointIndex = 0; ulPolyIndex < ulNumberOfPolylines; ulPolyIndex++)
			{
				unsigned int ulCurrentPolylinePointsCount = pPolylinePointCount[ulPolyIndex];
				if (0 == ulCurrentPolylinePointsCount)
					continue;

				T oPoint;
				m_oStream >> oPoint;
				MoveTo(oPoint);

				for (unsigned int ulPointIndex = 1; ulPointIndex < ulCurrentPolylinePointsCount; ulPointIndex++)
				{
					unsigned int ulRealPointIndex = ulPointIndex + ulStartPointIndex;
					if (ulRealPointIndex >= ulTotalPointsCount)
					{
						delete[] pPolylinePointCount;
						return SetError();
					}

					m_oStream >> oPoint;
					LineTo(oPoint);
				}				
			}
			DrawPath(true, false);

			delete[] pPolylinePointCount;
		}
		void Read_EMR_POLYTEXTOUTA()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfPolyTextoutA oText;
			m_oStream >> oText;
			
			if (0 == oText.cStrings)
				return;

			if (!oText.aEmrText)
				return SetError();

			for (unsigned int unIndex = 0; unIndex < oText.cStrings; unIndex++)
			{
				DrawTextA(oText.aEmrText[unIndex]);
			}
		}
		void Read_EMR_POLYTEXTOUTW()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfPolyTextoutW oText;
			m_oStream >> oText;

			if (0 == oText.cStrings)
				return;

			if (!oText.wEmrText)
				return SetError();

			for (unsigned int unIndex = 0; unIndex < oText.cStrings; unIndex++)
			{
				DrawTextA(oText.wEmrText[unIndex]);
			}
		}
		void Read_EMR_RECTANGLE()
		{
			TEmfRectL oBox;
			m_oStream >> oBox;

			MoveTo(oBox.lLeft, oBox.lTop);
			LineTo(oBox.lRight, oBox.lTop);
			LineTo(oBox.lRight, oBox.lBottom);
			LineTo(oBox.lLeft, oBox.lBottom);
			ClosePath();
			DrawPath(true, true);
		}
		void Read_EMR_ROUNDRECT()
		{
			TEmfRectL oBox;
			TEmfSizeL oCorner;
			m_oStream >> oBox >> oCorner;

			int lBoxW = oBox.lRight - oBox.lLeft;
			int lBoxH = oBox.lBottom - oBox.lTop;

			int lRoundW = (std::min)((int)oCorner.cx, lBoxW / 2);
			int lRoundH = (std::min)((int)oCorner.cy, lBoxH / 2);

			MoveTo(oBox.lLeft + lRoundW, oBox.lTop);
			LineTo(oBox.lRight - lRoundW, oBox.lTop);
			ArcTo(oBox.lRight - lRoundW, oBox.lTop, oBox.lRight, oBox.lTop + lRoundH, -90, 90);
			LineTo(oBox.lRight, oBox.lBottom - lRoundH);
			ArcTo(oBox.lRight - lRoundW, oBox.lBottom - lRoundH, oBox.lRight, oBox.lBottom, 0, 90);
			LineTo(oBox.lLeft + lRoundW, oBox.lBottom);
			ArcTo(oBox.lLeft, oBox.lBottom - lRoundH, oBox.lLeft + lRoundW, oBox.lBottom, 90, 90);
			LineTo(oBox.lLeft, oBox.lTop + lRoundH);
			ArcTo(oBox.lLeft, oBox.lTop, oBox.lLeft + lRoundW, oBox.lTop + lRoundW, 180, 90);
			ClosePath();
			DrawPath(true, true);
		}
		void Read_EMR_SETPIXELV()
		{
			// TODO: ��� �������� ����� ��������� ������ ������.
			TEmfPointL oPoint;
			TEmfColor oColor;

			m_oStream >> oPoint;
			m_oStream >> oColor;

			// ������ ������ �����
			BYTE pBgraBuffer[4];
			pBgraBuffer[0] = oColor.b;
			pBgraBuffer[1] = oColor.g;
			pBgraBuffer[2] = oColor.r;
			pBgraBuffer[3] = 0xff;

			m_pOutput->DrawBitmap(oPoint.x, oPoint.y, oPoint.x + 1, oPoint.y + 1, pBgraBuffer, 1, 1);
		}
		void Read_EMR_SMALLTEXTOUT()
		{
			TEmfSmallTextout oText;
			m_oStream >> oText;

			// ��������� oText � TEmfEmrText
			TEmfEmrText oEmrText;
			oEmrText.Chars        = oText.cChars;
			oEmrText.offDx        = 0;
			oEmrText.offString    = 0;
			oEmrText.Options      = oText.fuOptions;
			oEmrText.OutputString = oText.TextString;
			oEmrText.Reference.x  = oText.x;
			oEmrText.Reference.y  = oText.y;
			oEmrText.OutputDx     = NULL;

			// ������ �� ���������������� ���������, �������� ��������� ���� � �����, ��������� ������.
			unsigned int unSize = oText.GetSize();
			if (m_ulRecordSize - unSize > 0)
				m_oStream.Skip(m_ulRecordSize - unSize);
			else if (m_ulRecordSize - unSize < 0)
				m_oStream.SeekBack(unSize - m_ulRecordSize);

			DrawTextW(oEmrText);

			// ��������� �� ������ ����������� ������ �� ������, � �� ����������� ���� ������ �������� �����, ������ 
			// ��� �� ����������� ��������� ������������� ������.
			oEmrText.OutputString = NULL;
		}
		void Read_EMR_STROKEANDFILLPATH()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;
			if (m_pOutput && m_pPath)
			{
				m_pPath->Draw(m_pOutput, true, true);
				RELEASEOBJECT(m_pPath);
			}
		}
		void Read_EMR_STROKEPATH()
		{
			TEmfRectL oBounds;
			m_oStream >> oBounds;
			if (m_pOutput && m_pPath)
			{
				m_pPath->Draw(m_pOutput, true, false);
				RELEASEOBJECT(m_pPath);
			}
		}


	private:

		CDataStream       m_oStream;
		BYTE*             m_pBufferData;
		bool              m_bError;
		CFontManager*     m_pFontManager;
		TEmfHeader        m_oHeader;

		unsigned int     m_ulRecordSize;
		CEmfOutputDevice* m_pOutput;

		CEmfDC*           m_pDC;
		CEmfPlayer        m_oPlayer;

		CEmfPath*         m_pPath;

		int              m_lTest;

		friend class CEmfRendererOutput;
		friend class CEmfPlayer;
	};
}

#endif // _EMF_FILE_H