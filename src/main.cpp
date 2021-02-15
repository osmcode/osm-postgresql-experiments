
#include "options.hpp"
#include "table.hpp"

#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/diff_handler.hpp>
#include <osmium/diff_visitor.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/index/node_locations_map.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/util/verbose_output.hpp>
#include <osmium/visitor.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <string>
#include <utility>
#include <vector>

Options opts;

class Handler : public osmium::handler::Handler {

    std::vector<std::unique_ptr<Table>>& m_tables;

public:

    explicit Handler(std::vector<std::unique_ptr<Table>>& tables) :
        m_tables(tables) {
    }

    void osm_object(const osmium::OSMObject& object) {
        if (opts.filter_with_tags && object.tags().empty()) {
            return;
        }
        for (auto& table : m_tables) {
            if (table->matches(object.type())) {
                table->add_row(object, osmium::Timestamp{});
                table->possible_flush();
            }
        }
    }

    void changeset(const osmium::Changeset& changeset) {
        for (auto& table : m_tables) {
            if (table->matches(changeset.type())) {
                table->add_changeset_row(changeset);
                table->possible_flush();
            }
        }
    }

}; // class Handler

class DiffHandler : public osmium::diff_handler::DiffHandler {

    std::vector<std::unique_ptr<Table>>& m_tables;

    void osm_object(const osmium::OSMObject& object, const osmium::Timestamp next_version_timestamp) {
        if (opts.filter_with_tags && object.tags().empty()) {
            return;
        }
        for (auto& table : m_tables) {
            if (table->matches(object.type())) {
                table->add_row(object, next_version_timestamp);
                table->possible_flush();
            }
        }
    }

public:

    explicit DiffHandler(std::vector<std::unique_ptr<Table>>& tables) :
        m_tables(tables) {
    }

    void node(const osmium::DiffNode& dnode) {
        osmium::Timestamp ts;
        if (!dnode.last()) {
            ts = dnode.next().timestamp();
        }
        osm_object(dnode.curr(), ts);
    }

    void way(const osmium::DiffWay& dway) {
        osmium::Timestamp ts;
        if (!dway.last()) {
            ts = dway.next().timestamp();
        }
        osm_object(dway.curr(), ts);
    }

    void relation(const osmium::DiffRelation& drelation) {
        osmium::Timestamp ts;
        if (!drelation.last()) {
            ts = drelation.next().timestamp();
        }
        osm_object(drelation.curr(), ts);
    }

}; // class DiffHandler

void parse_command_line(int argc, char* argv[], std::string& input_filename, std::vector<std::unique_ptr<Table>>& tables) {
    po::options_description desc{"OPTIONS"};

    desc.add_options()
        ("filter,f", po::value<std::vector<std::string>>(), "Filter")
        ("help,h", "Show usage help")
        ("verbose,v", "Set verbose mode")
        ("with-history,H", "With history")
        ("add,a", "Add to existing Table")
    ;

    po::options_description hidden;
    hidden.add_options()
        ("input-filename", po::value<std::string>(), "Input file")
        ("tables", po::value<std::vector<std::string>>(), "Output tables")
    ;

    po::positional_options_description positional;
    positional.add("input-filename", 1);
    positional.add("tables", -1);

    po::options_description all;
    all.add(desc).add(hidden);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all).positional(positional).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << "Usage: " << argv[0] << " [OPTIONS] OSMFILE OUTPUT-TABLE...\n\n";
        std::cout << "OUTPUT-TABLE: Format: [FILENAME]=[STREAM]%[COLUMNS]\n";
        std::cout << "  FILENAME - output filename (leave empty for STDOUT)\n";
        std::cout << "  STREAM   - one of the following:\n";

        print_streams();

        std::cout << "  COLUMNS  - columns for this table (uses default when empty)\n";
        std::cout << '\n';
        std::cout << desc;
        std::exit(0);
    }

    if (vm.count("verbose")) {
        opts.verbose = true;
    }

    if (vm.count("with-history")) {
        opts.with_history = true;
    }

    if (vm.count("add")) {
        opts.add = true;
    }

    if (vm.count("filter")) {
        const auto filter = vm["filter"].as<std::vector<std::string>>();
        for (const auto& f : filter) {
            if (f == "with-tags") {
                opts.filter_with_tags = true;
            } else {
                std::cerr << "Warning! Unknown filter option: " << f << '\n';
            }
        }
    }

    if (vm.count("input-filename")) {
        input_filename = vm["input-filename"].as<std::string>();
    }

    if (vm.count("tables")) {
        for (const auto& table_config : vm["tables"].as<std::vector<std::string>>()) {
            tables.emplace_back(create_table(opts, table_config));
            const auto& new_table = *tables.back();
            if (!new_table.filename().empty()) {
                new_table.sql_data_definition();
            }
            if (new_table.column_flags() & sql_column_config_flags::location_store) {
                opts.use_location_handler = true;
            }
            if (new_table.column_flags() & sql_column_config_flags::assemble_areas) {
                opts.assemble_areas = true;
            }
            if (new_table.column_flags() & sql_column_config_flags::time_range) {
                opts.use_diff_handler = true;
            }
            if (opts.use_location_handler && opts.use_diff_handler) {
                throw std::runtime_error{"Can't use time ranges and geometries together"};
            }
        }
    } else {
        throw std::runtime_error{"No output tables found"};
    }
}

int main(int argc, char* argv[]) {
    std::vector<std::unique_ptr<Table>> tables;
    std::string input_filename{"-"};

    try {
        parse_command_line(argc, argv, input_filename, tables);
    } catch (const boost::program_options::error& e) {
        std::cerr << "Error parsing command line: " << e.what() << '\n';
        return 2;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 2;
    }

    osmium::VerboseOutput vout{opts.verbose};

    vout << "Options:\n";
    vout << "  With history: " << yes_no(opts.with_history);
    vout << "  Use diff handler: " << yes_no(opts.use_diff_handler);
    vout << "  Use location index: " << yes_no(opts.use_location_handler);
    vout << "  Assemble areas: " << yes_no(opts.assemble_areas);
    vout << " Add to existing Table: " << yes_no(opts.add);

    vout << "Filter:\n";
    vout << "  With tags: " << yes_no(opts.filter_with_tags);

    vout << "Tables:\n";

    osmium::osm_entity_bits::type read_entities = opts.use_location_handler ?
        osmium::osm_entity_bits::node : osmium::osm_entity_bits::nothing;
    for (const auto& table : tables) {
        if (table->read_entities() == osmium::osm_entity_bits::area) {
            read_entities |= osmium::osm_entity_bits::nwr;
        } else {
            read_entities |= table->read_entities();
        }
        vout << "  " << table->name() << ":\n";
        vout << "    filename: " << table->filename()       << '\n';
        vout << "    stream:   " << table->stream_name()    << '\n';
        vout << "    columns:  " << table->columns_string() << '\n';
    }

    // Normally a "users" table is only filled with the users seen in the
    // other tables that are specified. But if the "users" table is the only
    // one, we export all users.
    if (tables.size() == 1 && dynamic_cast<UsersTable*>(tables.front().get())) {
        read_entities = osmium::osm_entity_bits::all;
    }

    try {

        if (read_entities == osmium::osm_entity_bits::nothing) {
            throw std::runtime_error{"Nothing to do"};
        }

        vout << "Reading entities: " << list_entities(read_entities) << '\n';

        vout << "Transforming data...\n";

        osmium::io::File input_file{input_filename};

        if (opts.use_diff_handler) {
            DiffHandler handler{tables};
            osmium::io::Reader reader{input_file, read_entities};
            osmium::apply_diff(reader, handler);
            reader.close();
        } else {
            using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;
            using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;
            Handler handler{tables};
            if (opts.assemble_areas) {
                osmium::area::Assembler::config_type assembler_config;
                osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{assembler_config};
                vout << "First pass reading relations...\n";
                osmium::relations::read_relations(input_file, mp_manager);
                vout << "First pass done.\n";
                osmium::relations::print_used_memory(std::cerr, mp_manager.used_memory());
                index_type index;
                location_handler_type location_handler{index};
                location_handler.ignore_errors();
                vout << "Second pass...\n";
                osmium::io::Reader reader{input_file, read_entities};
                osmium::apply(reader, location_handler, mp_manager.handler([&handler](osmium::memory::Buffer&& buffer) {
                    osmium::apply(buffer, handler);
                }));
                reader.close();
                vout << "Second pass done.\n";
            } else if (opts.use_location_handler) {
                index_type index;
                location_handler_type location_handler{index};
                osmium::io::Reader reader{input_file, read_entities};
                osmium::apply(reader, location_handler, handler);
                reader.close();
            } else {
                osmium::io::Reader reader{input_file, read_entities};
                osmium::apply(reader, handler);
                reader.close();
            }
        }

        for (auto& table : tables) {
            table->flush();
            table->close();
        }

    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    vout << "Done.\n";
}

