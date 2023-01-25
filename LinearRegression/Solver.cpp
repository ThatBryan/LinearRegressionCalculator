#include "CSVParser.h"
#include <Eigen/Dense>
#include <iostream>
#include <vector>


class Solver
{
private:

	struct RegressionCoefficients
	{
		std::vector<double> coefficients;
	};

public:
	void SolveCoefficients(std::string_view depedentVarName)
	{
		// Y = XA + E
		// Y = dependent variable,
		// X = independent variables
		// A = Coefficient regressions

		const size_t numIndepedentVars = m_Parser.getNumColumns() - 1;
		const size_t numData = m_Parser.getNumRows();
		const auto prices = m_Parser.getColumnValueAsDouble(depedentVarName.data());
		auto headers = m_Parser.getColumnHeaders();

		std::erase_if(headers, [=](const std::string& s) { return s == depedentVarName; });

		Eigen::MatrixXd	y(numData, 1);
		for (size_t i = 0; i < numData; ++i)
			y(i, 0) = prices[i];
		
		std::vector<std::vector<double>> vars;
		for (size_t i = 0; i < numIndepedentVars; ++i)
			vars.push_back(m_Parser.getColumnValueAsDouble(headers[i]));

		Eigen::MatrixXd x(numData, numIndepedentVars + 1);
		for (size_t i = 0; i < numData; ++i)
		{
			x(i, 0) = 1.0;
			for (size_t j = 0; j < numIndepedentVars; ++j)
			{
				double v = vars[j][i];
				x(i, j + 1) = vars[j][i];
			}
		}

		auto xTranpose = x.transpose();
		auto a = (xTranpose * x).inverse() * xTranpose * y;

		m_RegressionCoefficients.resize(a.rows());
		for (size_t i = 0; i < m_RegressionCoefficients.size(); ++i)
			m_RegressionCoefficients[i] = a(i, 0);
	
	}

	void ReadCSVFile(std::string_view fileName)
	{
		m_Parser.SetCSVFilePath(fileName);
		m_Parser.Load();
	}

	double Predict(const std::vector<double>& independentVars)
	{

		if (m_RegressionCoefficients.size() < 2)
			throw std::runtime_error("Insufficient data to make predictions!");

		if (independentVars.size() != m_RegressionCoefficients.size() - 1)
			throw std::runtime_error("Mismatch counts of input and expected number independent variables!");

		// First will always be y-intercept
		double out = m_RegressionCoefficients.front();
		for (size_t i = 0; i < independentVars.size(); ++i)
		{
			out += m_RegressionCoefficients[i + 1] * independentVars[i];
		}

		return out;
	}

private:
	std::vector<double> m_RegressionCoefficients{};
	Utils::CSVParser m_Parser;
};



int main()
{
	Solver s;
	//s.ReadCSVFile("sample.csv");
	//double v = s.Predict("y", { 250.0, 85.0, 21.0 });

	s.ReadCSVFile("Gamer.csv");
	s.SolveCoefficients("ActionLatency");

	Utils::CSVParser unlabeledData{ "NewUnlabeledData.csv" };

	for (size_t i = 0; i < unlabeledData.getNumRows(); ++i)
	{
		std::cout << "The predicted value is: " << s.Predict(unlabeledData.getRowValuesAsDoubles(i)) << '\n';
	}
}