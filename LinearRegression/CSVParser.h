#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cassert>

namespace Utils
{

	class CSVParser
	{
	public:

		CSVParser() = default;
		CSVParser(std::string_view csvFileName) : m_CsvDirectory{ csvFileName }, maxWidth{0}
		{
			Load();
		}

		inline void SetCSVFilePath(std::string_view v) noexcept { m_CsvDirectory = v; }
		inline size_t getNumColumns() const noexcept { return m_Table.size(); }
		inline size_t getNumRows() const noexcept { return numRows; }
		inline const std::vector<std::string>& getColumnHeaders() const noexcept { return m_Headers; }
		std::vector<double> getRowValuesAsDoubles(size_t rowIndex) const noexcept
		{
			std::vector<double> vs{};

			for (size_t i = 0; i < m_Headers.size(); ++i)
			{
				const auto& data = m_Table.at(m_Headers[i]);
				assert(rowIndex < data.size());
				vs.emplace_back(std::stod(data[rowIndex]));
			}

			return vs;
		}


		[[nodiscard]] const std::vector<std::string>& getColumnValues(const std::string& columnName) const noexcept { return m_Table.at(columnName); };

		void PrintTable()
		{
			size_t iter = 0;
			const size_t width = maxWidth + 1;

			for (const auto& [key, values] : m_Table)
			{
				std::cout.width(width);
				std::cout << std::left << key;
				iter = std::max(iter, values.size());
			}

			std::cout << '\n' << '\n';


			for (size_t i = 0; i < iter; ++i)
			{
				for (const auto& [key, values] : m_Table)
				{
					std::cout.width(width);
					if (i <= values.size())
					{
						if (values[i].empty())
							std::cout << std::left << "(NULL)";
						else
							std::cout << std::left << values[i];
					}
				}

				std::cout << '\n';
			}
		}



		std::vector<double> getColumnValueAsDouble(const std::string& columnHeader) const noexcept
		{
			const std::vector<std::string>& col = m_Table.at(columnHeader);
			const size_t size = col.size();

			std::vector<double> vs(size, 0.0);

			for (size_t i = 0; i < size; ++i)
				vs[i] = std::stod(col[i]);
			
			return vs;
		}

		std::vector<int> getColumnValueAsInt(const std::string& columnHeader) const noexcept
		{
			const std::vector<std::string>& col = m_Table.at(columnHeader);
			const size_t size = col.size();

			std::vector<int> vs(size, 0.0);

			for (size_t i = 0; i < size; ++i)
				vs[i] = std::stoi(col[i]);

			return vs;
		}
		void Load()
		{
			if (!std::filesystem::exists(m_CsvDirectory))
				throw std::runtime_error("Unable to locate csv directory: %s");

			std::ifstream ifs{ m_CsvDirectory };

			if(!ifs.is_open())
				throw std::runtime_error("Unable to Open CSV File: \n");

			// Get name of headers first.
			std::string stringBuffer{};
			std::getline(ifs, stringBuffer);

			size_t prevCommaIndex = 0;
			size_t commaIndex = stringBuffer.find_first_of(",");


			if (commaIndex == std::string::npos)
				throw std::runtime_error("Format of csv file is unexpected, could not locate comma!\n");


			while (commaIndex != std::string::npos)
			{
				// TODO: If no header name?
				m_Table.insert({ stringBuffer.substr(prevCommaIndex, commaIndex - prevCommaIndex), {} });
				maxWidth = std::max(maxWidth, commaIndex - prevCommaIndex);
				m_Headers.emplace_back(std::move(stringBuffer.substr(prevCommaIndex, commaIndex - prevCommaIndex)));
				prevCommaIndex = commaIndex + 1;
				commaIndex = stringBuffer.find_first_of(",\n", prevCommaIndex);
			}

			// Account for last column, but is this always the case?
			commaIndex = stringBuffer.size();
			m_Table.insert({ stringBuffer.substr(prevCommaIndex, commaIndex - prevCommaIndex), {} });
			maxWidth = std::max(maxWidth, commaIndex - prevCommaIndex);
			m_Headers.emplace_back(std::move(stringBuffer.substr(prevCommaIndex, commaIndex - prevCommaIndex)));

			while (!ifs.eof())
			{
				std::getline(ifs, stringBuffer);
				if (stringBuffer.empty())
					continue;

				prevCommaIndex = 0;
				for (size_t i = 0; i < m_Table.size(); ++i)
				{
					commaIndex = stringBuffer.find_first_of(",", prevCommaIndex);
					commaIndex = std::min(commaIndex, stringBuffer.size());
					m_Table.at(m_Headers[i]).emplace_back(stringBuffer.substr(prevCommaIndex, commaIndex - prevCommaIndex));
					maxWidth = std::max(maxWidth, commaIndex - prevCommaIndex);
					prevCommaIndex = commaIndex + 1;
				}
			}
			ifs.close();

			numRows = m_Table.at(m_Headers.front()).size();
		}

	private:
		std::string m_CsvDirectory{};
		std::size_t maxWidth{0};
		std::size_t numRows{0};
		std::unordered_map<std::string, std::vector<std::string>> m_Table{}; // This should really be a vector
		std::vector<std::string> m_Headers{};
	};

}
