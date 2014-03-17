//----------------------------------------------------------------------------
/// \file  variant_tree_parser.hpp
/// \author Serge Aleynikov
//----------------------------------------------------------------------------
/// \brief Stream/file reader/writer of the variant tree class.
//----------------------------------------------------------------------------
// Created: 2014-02-22
//----------------------------------------------------------------------------
/*
***** BEGIN LICENSE BLOCK *****

This file is part of the utxx open-source project.

Copyright (C) 2014 Serge Aleynikov <saleyn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***** END LICENSE BLOCK *****
*/

#ifndef _UTXX_VARIANT_TREE_PARSER_HPP_
#define _UTXX_VARIANT_TREE_PARSER_HPP_

#include <utxx/variant_tree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <utxx/detail/variant_tree_scon_parser.hpp>

namespace utxx {

    template <class Ch>
    struct scon_writer_settings {
        scon_writer_settings
        (
            unsigned    a_tab_width       = 2,
            bool        a_show_types      = false,
            bool        a_show_braces     = true,
            Ch          a_indent_char     = Ch(' ')
        ) :
            tab_width(a_tab_width),
            show_types(a_show_types),
            show_braces(a_show_braces),
            indent_char(a_indent_char)
        {}
        const int  tab_width;
        const bool show_types;
        const bool show_braces;
        const Ch   indent_char;
    };

    /**
     * Basic resolver of files in included in SCON format via the
     * '#include "filename.config"' clause.  The instance of this class
     * takes a list of directories, and finds a file with name \a a_filename
     * in the filesystem under one of the given directories.
     */
    template <class Ch>
    class inc_file_resolver {
        typedef std::basic_string<Ch> string;
        std::vector<string> m_dirs;

    public:
        explicit inc_file_resolver
        (
            const std::vector<string>& a_dirs = std::vector<string>()
        )
            : m_dirs(a_dirs)
        {}

        /// Main resolving functor
        bool operator() (string& a_filename) {
            string name = boost::filesystem::path(a_filename).is_absolute()
                        ? boost::filesystem::basename(a_filename)
                        : a_filename;
            for (typename std::vector<string>::const_iterator
                    it = m_dirs.begin(), e = m_dirs.end(); it != e; ++it)
            {
                boost::filesystem::path inc_file =
                    boost::filesystem::path(*it) / name;
                if (boost::filesystem::exists(inc_file)) {
                    a_filename = inc_file.string();
                    return true;
                }
            }
            return false;
        }
    };

    /**
     * Read SCON format from a the given stream and translate it to a variant tree.
     * @param a_stream is an input stream
     * @param a_filename is a filename associated with stream in case of exceptions
     * @param a_inc_filename_resolver is the resolver of files included in the
     *                                scon configuration via '#include "filename"'
     *                                clause
     * @note Replaces the existing contents. Strong exception guarantee.
     * @throw file_parser_error If the stream cannot be read, doesn't contain
     *                          valid SCON, or a conversion fails.
     */
    template<class Ch>
    void read_scon
    (
        std::basic_istream<Ch>& a_stream,
        basic_variant_tree<Ch>& a_tree,
        const std::string&      a_filename  = std::string(),
        const boost::function<bool (std::string& a_filename)>
                                a_inc_filename_resolver = inc_file_resolver<char>()
    )
    {
        typedef typename basic_variant_tree<Ch>::translator_from_string     translator;
        typedef detail::scon_reader<basic_variant_tree<Ch>, translator, Ch> scon_reader;
        translator              tr;
        basic_variant_tree<Ch>  tree;
        int                     lineno  = 0;
        const Ch*               text    = NULL;
        std::basic_string<Ch>   line;
        scon_reader rd
        (
            a_stream, tree, a_filename, lineno, line, 0,
            tr, text, a_inc_filename_resolver
        );
        a_tree.swap(tree);
    }

    /**
     * Read SCON from a the given file and translate it to a variant tree. The
     * tree's key type must be a string type, i.e. it must have a nested
     * value_type typedef that is a valid parameter for basic_ifstream.
     * @param a_filename is a filename associated with stream in case of exceptions
     * @param a_tree is the destination of configuration data
     * @param a_dirs is a vector of directories to use for resolving included files
     * @param a_loc is the locale to use
     * @note Replaces the existing contents. Strong exception guarantee.
     * @throw info_parser_error If the file cannot be read, doesn't contain
     *                          valid INFO, or a conversion fails.
     */
    template <class Ch>
    void read_scon
    (
        const std::string&      a_filename,
        basic_variant_tree<Ch>& a_tree,
        const boost::function<bool (std::string& a_filename)>
                                a_inc_filename_resolver = inc_file_resolver<char>(),
        const std::locale&      a_loc   = std::locale()
    )
    {
        std::basic_ifstream<Ch> stream(a_filename.c_str());
        if (!stream) {
            BOOST_PROPERTY_TREE_THROW(
                boost::property_tree::file_parser_error(
                    "cannot open file for reading", a_filename, 0));
        }
        stream.imbue(a_loc);
        read_scon(stream, a_tree, a_filename, a_inc_filename_resolver);
    }


    /**
     * Writes a tree to the stream in SCON format.
     * @throw file_parser_error If the stream cannot be written to, or a
     *                          conversion fails.
     * @param settings The settings to use when writing the SCON data.
     */
    template <class Stream, class Ch>
    void write_scon
    (
        Stream&                         a_stream,
        const basic_variant_tree<Ch>&   a_tree,
        const scon_writer_settings<Ch>& a_settings = scon_writer_settings<Ch>()
    ) {
        a_tree.dump
        (
            a_stream,
            a_settings.tab_width,
            a_settings.show_types,
            a_settings.show_braces,
            a_settings.indent_char
        );
    }

    /**
     * Writes a tree to the file in SCON format. The tree's key type must be a
     * string type, i.e. it must have a nested value_type typedef that is a
     * valid parameter for basic_ofstream.
     * @throw file_parser_error If the file cannot be written to, or a
     *                          conversion fails.
     * @param settings The settings to use when writing the INFO data.
     */
    template <class Ch>
    void write_scon
    (
        const std::string &             a_filename,
        const basic_variant_tree<Ch>&   a_tree,
        const scon_writer_settings<Ch>& a_settings  = scon_writer_settings<Ch>(),
        const std::locale &             a_loc       = std::locale()
    )
    {
        std::basic_ofstream<Ch> stream(a_filename.c_str());
        if (!stream) {
            throw boost::property_tree::file_parser_error(
                    "cannot open file for writing", a_filename, 0);
        }
        stream.imbue(a_loc);
        write_scon(stream, a_tree, a_settings);
        if (!stream.good())
            throw boost::property_tree::file_parser_error(
                    "write error", a_filename, 0);
    }

    /**
     * Read INFO format from a the given stream and translate it to a variant tree.
     * @param a_stream is an input stream
     * @note Replaces the existing contents. Strong exception guarantee.
     * @throw file_parser_error If the stream cannot be read, doesn't contain
     *                          valid SCON, or a conversion fails.
     */
    template <typename Source, typename Ch>
    void read_info(Source& a_src, basic_variant_tree<Ch>& a_tree) {
        typename basic_variant_tree<Ch>::base& pt =
            static_cast<typename basic_variant_tree<Ch>::base&>(a_tree);
        boost::property_tree::info_parser::read_info(a_src, pt);
        boost::property_tree::translator_between<utxx::variant, std::string> tr;
        basic_variant_tree<Ch>::translate_data(a_tree, tr);
    }

    template <typename Target, typename Ch>
    void write_info(Target& a_tar, basic_variant_tree<Ch>& a_tree) {
        boost::property_tree::info_parser::write_info(a_tar, a_tree);
    }

    template <typename Target, typename Ch, typename Settings>
    void write_info
    (
        Target& a_tar, basic_variant_tree<Ch>& a_tree, const Settings& a_settings
    ) {
        boost::property_tree::info_parser::write_info(
            a_tar, a_tree, a_settings);
    }

    /// @brief Read configuration from an XML file
    /// @param a_src   is the configuration source (file or stream
    /// @param a_tree  target configuration tree
    /// @param a_flags are optional flags
    ///                (see boost::property_tree::xml_parser::read_xml())
    template <typename Source, typename Ch>
    void read_xml(Source& a_src, basic_variant_tree<Ch>& a_tree, int a_flags = 0)
    {
        // TODO: Implement native support of read_xml for variant_tree instead
        //       of using this workaround of reading to ptree and copying.
        typename basic_variant_tree<Ch>::base& pt =
            static_cast<typename basic_variant_tree<Ch>::base&>(a_tree);
        boost::property_tree::xml_parser::read_xml(a_src, pt, a_flags);
        boost::property_tree::translator_between<utxx::variant, std::string> tr;
        basic_variant_tree<Ch>::translate_data(a_tree, tr);
    }

    template <typename Source, typename Ch>
    void  read_ini(Source& a_src, basic_variant_tree<Ch>& a_tree, int a_flags = 0)
    {
        // TODO: Implement native support of read_xml for variant_tree instead
        //       of using this workaround of reading to ptree and copying.
        typename basic_variant_tree<Ch>::base& pt =
            static_cast<typename basic_variant_tree<Ch>::base&>(a_tree);
        boost::property_tree::ini_parser::read_ini(a_src, pt);
        boost::property_tree::translator_between<utxx::variant, std::string> tr;
        basic_variant_tree<Ch>::translate_data(a_tree, tr);
    }

    /*
    template <typename Target>
    void write_xml(Target& a_tar, variant_tree& a_tree) {
        boost::property_tree::xml_parser::write_xml(
            a_tar, static_cast<detail::basic_variant_tree&>(a_tree));
    }

    template <typename Target, typename Settings>
    void write_xml(Target& a_tar, variant_tree& a_tree, const Settings& a_settings) {
        boost::property_tree::xml_parser::write_xml(
            a_tar, static_cast<detail::basic_variant_tree&>(a_tree), a_settings);
    }
    */

    /**
     * @brief Read SCON/INI/XML/INFO file format by guessing content type by extension
     * @param a_filename is a filename associated with stream in case of exceptions
     * @param a_tree     is the tree
     * @param a_inc_filename_resolver is the resolver of files included in the
     *                                configuration via '#include "filename"'
     *                                clause (given that configuration format, such
     *                                as SCON supports this feature)
     * @param a_flags are the optional flags for reading XML file
     *                (see boost::property_tree::xml_parser::read_xml())
     * @note Replaces the existing contents. Strong exception guarantee.
     * @throw file_parser_error If the stream cannot be read, doesn't contain
     *                          valid format, or a conversion fails.
     */
    template<class Ch>
    void read_config_file
    (
        const std::string&      a_filename,
        basic_variant_tree<Ch>& a_tree,
        const boost::function<bool (std::string& a_filename)>
                                a_inc_filename_resolver = inc_file_resolver<char>(),
        int                     a_flags = 0
    ) {
        std::string ext = boost::filesystem::extension(a_filename);
        if (ext == ".config" || ext == ".conf" || ext == ".cfg" || ext == ".scon")
            ::utxx::read_scon(a_filename, a_tree, a_inc_filename_resolver);
        else if (ext == ".ini")
            ::utxx::read_ini(a_filename, a_tree);
        else if (ext == ".xml")
            ::utxx::read_xml(a_filename, a_tree, a_flags);
        else
            throw std::runtime_error("Configuration file extension not supported!");
    }

} // namespace utxx

#endif // _UTXX__PARSER_HPP_