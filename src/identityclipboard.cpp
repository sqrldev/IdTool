/*
 * This file is part of the "IdTool" utility app.
 *
 * MIT License
 *
 * Copyright (c) 2019 Alexander Hauser
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "identityclipboard.h"

/*!
 *
 * \class IdentityClipboard
 * \brief A class for enabling copy-pasting of identity blocks.
 *
 * \c IdentityClipboard holds a single \c IdentityBlock which can be used
 * for copy-paste operations.
 *
 * The class adheres to the "singleton" pattern and can therefore not be
 * instantiated directly using the constructor. Instead, call
 * \c IdentityClipboard::getInstance() to get a pointer to the singleton
 * instance.
 *
*/


IdentityClipboard* IdentityClipboard::m_pInstance = nullptr;
IdentityClipboard::IdentityClipboard() { }
IdentityClipboard::~IdentityClipboard() { delete m_pInstance; }

/*!
 * Returns an \c IdentityClipboard instance while ensuring that only one
 * instance of the class exists at any given time.
 */

IdentityClipboard* IdentityClipboard::getInstance()
{
    if (m_pInstance == nullptr)
    {
        m_pInstance = new IdentityClipboard();
    }

    return m_pInstance;
}

/*!
 * Stores \a block in the clipboard so that it can be retrieved
 * later using \c getBlock().
 *
 * \sa getBlock()
 */

void IdentityClipboard::setBlock(IdentityBlock block)
{
    m_block = block;
    m_bHasBlock = true;
}

/*!
 * Retrieves an \c IdentityBlock from the clipboard. If no block was
 * previously stored within the clipboard using \c setBlock(), a
 * \c std::runtime_error is thrown.
 *
 * \sa setBlock(), hasBlock()
 */

IdentityBlock IdentityClipboard::getBlock()
{
    if (!hasBlock())
    {
        throw new std::runtime_error("No block present in clipboard!");
    }
    return m_block;
}

/*!
 * Returns \c true if an \c IdentityBlock is present within the clipboard
 * (was previously stored using \c setBlock()), and \c false otherwise.
 *
 * \sa setBlock(), getBlock()
 */

bool IdentityClipboard::hasBlock()
{
    return m_bHasBlock;
}
